package main

// Tunnel/OAuth gateway adapted from KagSSH-Go, without gradio-python or frpc.
import (
	"bytes"
	"context"
	"crypto/rand"
	"crypto/subtle"
	"encoding/base64"
	"errors"
	"flag"
	"fmt"
	"io"
	"log"
	"net"
	"net/http"
	"os"
	"os/exec"
	"os/signal"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
	"syscall"
	"time"

	"github.com/ThowiLabs/GPTFrames/tunnel/internal/gradiolive"
	"github.com/ThowiLabs/GPTFrames/tunnel/internal/oauth"
	"github.com/ThowiLabs/GPTFrames/tunnel/internal/pinauth"
)

const maxBody = 1 << 20

func randomString(n int) (string, error) {
	b := make([]byte, n)
	_, e := rand.Read(b)
	return base64.RawURLEncoding.EncodeToString(b), e
}
func storedPIN(dir string) (string, error) {
	if e := os.MkdirAll(dir, 0700); e != nil {
		return "", e
	}
	if e := os.Chmod(dir, 0700); e != nil {
		return "", e
	}
	file := filepath.Join(dir, "oauth-pin.txt")
	if raw, e := os.ReadFile(file); e == nil {
		return strings.TrimSpace(string(raw)), nil
	} else if !errors.Is(e, os.ErrNotExist) {
		return "", e
	}
	pin, e := pinauth.Generate()
	if e != nil {
		return "", e
	}
	f, e := os.OpenFile(file, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0600)
	if e != nil {
		return "", e
	}
	_, e = f.WriteString(pin + "\n")
	if e2 := f.Close(); e == nil {
		e = e2
	}
	return pin, e
}

type bridge struct {
	mu    sync.RWMutex
	next  http.Handler
	nonce string
}

func (b *bridge) set(h http.Handler) { b.mu.Lock(); b.next = h; b.mu.Unlock() }
func (b *bridge) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	if r.URL.Path == "/__gptframes_probe" && r.Method == http.MethodGet {
		w.Header().Set("Content-Type", "text/plain")
		w.Header().Set("Cache-Control", "no-store")
		fmt.Fprint(w, b.nonce)
		return
	}
	b.mu.RLock()
	h := b.next
	b.mu.RUnlock()
	if h == nil {
		http.Error(w, "MCP inicializando", 503)
		return
	}
	h.ServeHTTP(w, r)
}
func verifyPublic(ctx context.Context, link, nonce string) error {
	c := &http.Client{Timeout: 9 * time.Second, CheckRedirect: func(_ *http.Request, _ []*http.Request) error { return errors.New("redirección externa") }}
	var last error
	for i := 0; i < 7; i++ {
		req, e := http.NewRequestWithContext(ctx, "GET", link+"/__gptframes_probe", nil)
		if e != nil {
			return e
		}
		res, e := c.Do(req)
		if e == nil {
			data, e2 := io.ReadAll(io.LimitReader(res.Body, 256))
			res.Body.Close()
			if e2 == nil && res.StatusCode == 200 && subtle.ConstantTimeCompare(data, []byte(nonce)) == 1 {
				return nil
			}
			last = fmt.Errorf("probe HTTP %d", res.StatusCode)
		} else {
			last = e
		}
		select {
		case <-ctx.Done():
			return ctx.Err()
		case <-time.After(1500 * time.Millisecond):
		}
	}
	return fmt.Errorf("verificación FRP: %w", last)
}
func launchBackend(ctx context.Context, root string, port int) (string, error) {
	check, err := net.DialTimeout("tcp", "127.0.0.1:"+strconv.Itoa(port), 250*time.Millisecond)
	if err == nil {
		_ = check.Close()
		return "", fmt.Errorf("puerto backend %d en uso; no conectar con proceso ajeno", port)
	}

	secret, e := randomString(48)
	if e != nil {
		return "", e
	}
	cmd := exec.CommandContext(ctx, "python3", filepath.Join(root, "mcp/server.py"), "--transport", "http", "--port", strconv.Itoa(port))
	cmd.Dir = root
	cmd.Env = append(os.Environ(), "GPTFRAMES_MCP_TOKEN="+secret)
	cmd.Stdout = os.Stderr
	cmd.Stderr = os.Stderr
	if e = cmd.Start(); e != nil {
		return "", e
	}
	go func() { _ = cmd.Wait() }()
	for i := 0; i < 30; i++ {
		c := &http.Client{Timeout: 750 * time.Millisecond}
		res, err := c.Get("http://127.0.0.1:" + strconv.Itoa(port) + "/health")
		if err == nil {
			res.Body.Close()
			if res.StatusCode == 200 {
				return secret, nil
			}
		}
		select {
		case <-ctx.Done():
			return "", ctx.Err()
		case <-time.After(200 * time.Millisecond):
		}
	}
	return "", errors.New("MCP Python no responde")
}
func protectedProxy(auth *oauth.Server, port int, token string) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Cache-Control", "no-store")
		w.Header().Set("X-Content-Type-Options", "nosniff")
		if auth.ValidateBearer(r) != nil {
			auth.Challenge(w)
			return
		}
		if r.Method != "POST" {
			w.Header().Set("Allow", "POST")
			http.Error(w, "Solo POST MCP", 405)
			return
		}
		r.Body = http.MaxBytesReader(w, r.Body, maxBody)
		b, e := io.ReadAll(r.Body)
		if e != nil {
			http.Error(w, "MCP demasiado grande", 413)
			return
		}
		if len(b) == 0 {
			http.Error(w, "MCP vacío", 400)
			return
		}
		req, e := http.NewRequestWithContext(r.Context(), "POST", "http://127.0.0.1:"+strconv.Itoa(port)+"/mcp", bytes.NewReader(b))
		if e != nil {
			http.Error(w, "Error interno", 500)
			return
		}
		req.Header.Set("Authorization", "Bearer "+token)
		req.Header.Set("Content-Type", "application/json")
		res, e := (&http.Client{Timeout: 4 * time.Minute}).Do(req)
		if e != nil {
			http.Error(w, "MCP backend no disponible", 502)
			return
		}
		defer res.Body.Close()
		w.Header().Set("Content-Type", "application/json")
		w.WriteHeader(res.StatusCode)
		_, _ = io.Copy(w, io.LimitReader(res.Body, maxBody))
	})
}
func main() {
	var port, backend int
	var local bool
	var root, dir string
	flag.IntVar(&port, "port", 8780, "Gateway local port")
	flag.IntVar(&backend, "backend-port", 8769, "MCP backend port")
	flag.BoolVar(&local, "local", false, "No public Gradio tunnel")
	flag.StringVar(&root, "project-dir", "", "GPTFrames repo path")
	flag.StringVar(&dir, "data-dir", "", "Private OAuth state")
	flag.Parse()
	if port < 1024 || port > 65535 || backend < 1024 || backend > 65535 || port == backend {
		log.Fatal("Puertos inválidos")
	}
	if root == "" {
		var e error
		root, e = os.Getwd()
		if e != nil {
			log.Fatal(e)
		}
	}
	root, _ = filepath.Abs(root)
	if dir == "" {
		dir = filepath.Join(root, ".gptframes-private")
	}
	pin, e := storedPIN(dir)
	if e != nil {
		log.Fatal(e)
	}
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()
	nonce, e := randomString(18)
	if e != nil {
		log.Fatal(e)
	}
	b := &bridge{nonce: nonce}
	addr := "127.0.0.1:" + strconv.Itoa(port)
	ln, e := net.Listen("tcp", addr)
	if e != nil {
		log.Fatal(e)
	}
	defer ln.Close()
	srv := &http.Server{Handler: b, ReadHeaderTimeout: 8 * time.Second, ReadTimeout: 5 * time.Minute, WriteTimeout: 5 * time.Minute, MaxHeaderBytes: 16 << 10}
	go func() {
		if e := srv.Serve(ln); e != nil && !errors.Is(e, http.ErrServerClosed) {
			log.Printf("HTTP: %v", e)
			stop()
		}
	}()
	defer func() {
		c, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		_ = srv.Shutdown(c)
	}()
	public := "http://" + addr
	var tunnel *gradiolive.Tunnel
	if !local {
		tunnel, e = gradiolive.Start(ctx, port)
		if e != nil {
			log.Fatal("Gradio: ", e)
		}
		defer tunnel.Close()
		public = tunnel.URL
		if e = verifyPublic(ctx, public, nonce); e != nil {
			log.Fatal(e)
		}
	}
	token, e := launchBackend(ctx, root, backend)
	if e != nil {
		log.Fatal(e)
	}
	auth, e := oauth.New(oauth.Config{PublicURL: public, StateFile: filepath.Join(dir, "oauth-state.json"), AccessPIN: pin,
		AccessTTL: time.Hour, RefreshTTL: 30 * 24 * time.Hour, PINMaxAttempts: 5, PINWindow: 5 * time.Minute, PINLockout: 15 * time.Minute})
	if e != nil {
		log.Fatal("OAuth: ", e)
	}
	mux := http.NewServeMux()
	auth.RegisterRoutes(mux)
	mux.Handle("/mcp", protectedProxy(auth, backend, token))
	mux.HandleFunc("GET /health", func(w http.ResponseWriter, _ *http.Request) {
		w.Header().Set("Content-Type", "application/json")
		_, _ = io.WriteString(w, "{\"status\":\"ok\",\"auth\":\"oauth-pin\"}")
	})
	b.set(mux)
	fmt.Println("MCP URL:", public+"/mcp")
	fmt.Println("PIN stored privately at:", filepath.Join(dir, "oauth-pin.txt"))
	fmt.Println("To connect to ChatGPT, add an MCP app and authorize it with OAuth + your PIN.")
	if tunnel == nil {
		<-ctx.Done()
	} else {
		select {
		case <-ctx.Done():
		case <-tunnel.Done():
			log.Printf("Gradio cerrado: %v", tunnel.Err())
		}
	}
}
