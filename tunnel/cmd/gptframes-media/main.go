package main

// Servidor público de archivos; HTTP FileServer soporta Range para seek MP4.
// Mantiene el MCP OAuth en un proceso y puerto completamente distintos.
import (
	"flag"
	"log"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"time"
)

func main() {
	port := flag.Int("port", 18989, "Puerto loopback público")
	root := flag.String("root", "examples/go-rust-cpp-1080p/public", "Carpeta pública")
	flag.Parse()
	if *port < 1024 || *port > 65535 {
		log.Fatal("Puerto inválido")
	}
	path, err := filepath.Abs(*root)
	if err != nil {
		log.Fatal(err)
	}
	if st, err := os.Stat(path); err != nil || !st.IsDir() {
		log.Fatal("Carpeta no encontrada")
	}
	files := http.FileServer(http.Dir(path))
	handler := http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if r.Method != "GET" && r.Method != "HEAD" {
			w.Header().Set("Allow", "GET, HEAD")
			http.Error(w, "Método no permitido", 405)
			return
		}
		w.Header().Set("X-Content-Type-Options", "nosniff")
		w.Header().Set("Cache-Control", "public, max-age=120")
		files.ServeHTTP(w, r)
	})
	srv := &http.Server{Addr: net.JoinHostPort("127.0.0.1", strconv.Itoa(*port)), Handler: handler,
		ReadHeaderTimeout: 8 * time.Second, WriteTimeout: 5 * time.Minute, IdleTimeout: 60 * time.Second, MaxHeaderBytes: 16 << 10}
	log.Printf("GPTFrames media local port %d", *port)
	log.Fatal(srv.ListenAndServe())
}
