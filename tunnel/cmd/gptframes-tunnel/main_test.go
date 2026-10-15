package main

import (
	"context"
	"net"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func TestPINIsPrivateAndStable(t *testing.T) {
	dir := filepath.Join(t.TempDir(), "private")
	a, err := storedPIN(dir)
	if err != nil {
		t.Fatal(err)
	}
	b, err := storedPIN(dir)
	if err != nil {
		t.Fatal(err)
	}
	if a != b || len(a) < 8 {
		t.Fatal("PIN no persistente")
	}
	st, err := os.Stat(filepath.Join(dir, "oauth-pin.txt"))
	if err != nil {
		t.Fatal(err)
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
	l, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer l.Close()
	port := l.Addr().(*net.TCPAddr).Port
	_, err = launchBackend(context.Background(), t.TempDir(), port)
	if err == nil {
		t.Fatal("Gateway could attach to unrelated local service")
	}
}

func writeTestEnv(t *testing.T, root, value string, mode os.FileMode) {
	t.Helper()
	path := filepath.Join(root, ".env")
	if err := os.WriteFile(path, []byte(value), mode); err != nil {
		t.Fatal(err)
	}
	if err := os.Chmod(path, mode); err != nil {
		t.Fatal(err)
	}
}

func TestConfiguredPINFromDotenvOverridesGenerated(t *testing.T) {
	root := t.TempDir()
	dir := filepath.Join(root, ".gptframes-private")
	oldPIN, err := storedPIN(dir)
	if err != nil {
		t.Fatal(err)
	}
	custom := "MiPinPersonal2026-Privado"
	writeTestEnv(t, root, "# private\n"+oauthPINEnvName+"='"+custom+"'\n", 0600)
	actual, source, err := configuredPIN(root, dir)
	if err != nil {
		t.Fatal(err)
	}
	if actual != custom || !strings.Contains(source, ".env") {
		t.Fatalf("wrong PIN source (%s)", source)
	}
	if actual == oldPIN {
		t.Fatal("old random PIN still used")
	}
	existing, err := os.ReadFile(filepath.Join(dir, "oauth-pin.txt"))
	if err != nil {
		t.Fatal(err)
	}
	if strings.TrimSpace(string(existing)) != oldPIN {
		t.Fatal(".env overwrote legacy PIN file")
	}
}

func TestPINEnvironmentOverridesDotenv(t *testing.T) {
	root := t.TempDir()
	writeTestEnv(t, root, oauthPINEnvName+"=PINFromDotenv2026\n", 0600)
	t.Setenv(oauthPINEnvName, "PINFromShell2026")
	pin, source, err := configuredPIN(root, filepath.Join(root, "private"))
	if err != nil {
		t.Fatal(err)
	}
	if pin != "PINFromShell2026" || !strings.Contains(source, "variable de entorno") {
		t.Fatalf("wrong precedence, source=%s", source)
	}
	if _, err := os.Stat(filepath.Join(root, "private")); !os.IsNotExist(err) {
		t.Fatal("generated PIN file created unexpectedly")
	}
}

func TestPINPrivateAndValidityRules(t *testing.T) {
	cases := []struct {
		name, content string
		perm          os.FileMode
		wantErr       string
	}{
		{"quoted", oauthPINEnvName + "=\"Secure-PIN#2026\"\n", 0600, ""},
		{"inline comment", oauthPINEnvName + "=Secure-PIN2026 # comment\n", 0600, ""},
		{"empty", oauthPINEnvName + "=\n", 0600, "inválido"},
		{"short", oauthPINEnvName + "=12345\n", 0600, "inválido"},
		{"duplicate", oauthPINEnvName + "=FirstSecurePIN\n" + oauthPINEnvName + "=SecondSecurePIN\n", 0600, "duplicada"},
		{"insecure", oauthPINEnvName + "=SomeSecurePin123\n", 0644, "chmod 600"},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if tc.name == "insecure" && runtime.GOOS == "windows" {
				t.Skip("Windows utiliza ACL en vez de permisos POSIX")
			}
			root := t.TempDir()
			writeTestEnv(t, root, tc.content, tc.perm)
			_, _, err := configuredPIN(root, filepath.Join(root, "private"))
			if tc.wantErr != "" {
				if err == nil || !strings.Contains(err.Error(), tc.wantErr) {
					t.Fatalf("expected error containing %q, got %v", tc.wantErr, err)
				}
			} else if err != nil {
				t.Fatal(err)
			}
		})
	}
}

func TestPINFallbackWhenNoDotenv(t *testing.T) {
	root := t.TempDir()
	pin, source, err := configuredPIN(root, filepath.Join(root, "private"))
	if err != nil {
		t.Fatal(err)
	}
	if !strings.Contains(source, "generado automáticamente") || len(pin) < 6 {
		t.Fatalf("unexpected fallback source: %s", source)
	}
}

func TestPINMalformedEnvDoesNotFallBack(t *testing.T) {
	root := t.TempDir()
	writeTestEnv(t, root, oauthPINEnvName+"=\n", 0600)
	_, _, err := configuredPIN(root, filepath.Join(root, "private"))
	if err == nil {
		t.Fatal("empty .env PIN should not silently fall back to an old PIN")
	}
}

func TestPINDotenvSymlinkRejected(t *testing.T) {
	if runtime.GOOS == "windows" {
		t.Skip("file symlink policies differ on Windows")
	}
	root := t.TempDir()
	target := filepath.Join(root, "real.env")
	if err := os.WriteFile(target, []byte(oauthPINEnvName+"=ThisPINIsPrivate2026\n"), 0600); err != nil {
		t.Fatal(err)
	}
	if err := os.Symlink(target, filepath.Join(root, ".env")); err != nil {
		t.Fatal(err)
	}
	if _, _, err := configuredPIN(root, filepath.Join(root, "private")); err == nil {
		t.Fatal("symlink .env accepted")
	}
}

func TestPINEmptyProcessVariableRejected(t *testing.T) {
	root := t.TempDir()
	writeTestEnv(t, root, oauthPINEnvName+"=ThisWouldOtherwiseWork2026\n", 0600)
	t.Setenv(oauthPINEnvName, "")
	if _, _, err := configuredPIN(root, filepath.Join(root, "private")); err == nil {
		t.Fatal("empty explicit environment variable should not fall back to .env")
	}
}
