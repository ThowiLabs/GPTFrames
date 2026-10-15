package main

import (
	"context"
	"net"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"testing"
)

func TestPINIsPrivateAndStable(t *testing.T) {
	dir := filepath.Join(t.TempDir(), "private")
	a, e := storedPIN(dir)
	if e != nil {
		t.Fatal(e)
	}
	b, e := storedPIN(dir)
	if e != nil {
		t.Fatal(e)
	}
	if a != b || len(a) < 8 {
		t.Fatal("PIN no persistente")
	}
	st, e := os.Stat(filepath.Join(dir, "oauth-pin.txt"))
	if e != nil {
		t.Fatal(e)
	}
	if st.Mode().Perm() != 0600 {
		t.Fatalf("PIN no privado: %v", st.Mode())
	}
}
func TestGatewayFailsClosedUntilOAuthReady(t *testing.T) {
	b := &bridge{nonce: "nonce-for-test"}
	r := httptest.NewRequest("POST", "/mcp", nil)
	out := httptest.NewRecorder()
	b.ServeHTTP(out, r)
	if out.Code != http.StatusServiceUnavailable {
		t.Fatalf("unready gateway HTTP %d", out.Code)
	}
	b.set(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { w.WriteHeader(201) }))
	out = httptest.NewRecorder()
	b.ServeHTTP(out, r)
	if out.Code != 201 {
		t.Fatal("upstream handler not routed")
	}
	out = httptest.NewRecorder()
	b.ServeHTTP(out, httptest.NewRequest("GET", "/__gptframes_probe", nil))
	if out.Code != 200 || out.Body.String() != "nonce-for-test" {
		t.Fatal("probe mismatch")
	}
}
func TestBackendOccupiedNotReused(t *testing.T) {
	l, e := net.Listen("tcp", "127.0.0.1:0")
	if e != nil {
		t.Fatal(e)
	}
	defer l.Close()
	port := l.Addr().(*net.TCPAddr).Port
	_, e = launchBackend(context.Background(), t.TempDir(), port)
	if e == nil {
		t.Fatal("Gateway could attach to unrelated local service")
	}
}
