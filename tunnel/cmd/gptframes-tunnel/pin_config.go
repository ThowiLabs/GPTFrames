package main

// Custom OAuth PIN configuration. The private .env in the project directory
// overrides an older auto-generated PIN, but it is never copied to that file,
// passed to Python, printed in logs, or committed to source control.
import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/ThowiLabs/GPTFrames/tunnel/internal/pinauth"
)

const oauthPINEnvName = "GPTFRAMES_OAUTH_PIN"

// configuredPIN priority:
//  1. environment GPTFRAMES_OAUTH_PIN (useful for secret managers)
//  2. project-root .env variable GPTFRAMES_OAUTH_PIN
//  3. persisted generated PIN at dataDir/oauth-pin.txt (legacy fallback)
func configuredPIN(projectRoot, dataDir string) (pin, source string, err error) {
	if value, exists := os.LookupEnv(oauthPINEnvName); exists {
		if err := validateConfiguredPIN(value); err != nil {
			return "", "", err
		}
		return value, "variable de entorno " + oauthPINEnvName, nil
	}
	value, found, err := dotenvPIN(filepath.Join(projectRoot, ".env"))
	if err != nil {
		return "", "", err
	}
	if found {
		if err := validateConfiguredPIN(value); err != nil {
			return "", "", err
		}
		return value, "archivo .env del proyecto", nil
	}
	value, err = storedPIN(dataDir)
	if err != nil {
		return "", "", err
	}
	if err := pinauth.Validate(value); err != nil {
		return "", "", fmt.Errorf("PIN guardado inválido: %w", err)
	}
	return value, "archivo privado oauth-pin.txt (generado automáticamente)", nil
}

func validateConfiguredPIN(pin string) error {
	if err := pinauth.Validate(pin); err != nil {
		return fmt.Errorf("%s inválido: %w", oauthPINEnvName, err)
	}
	if strings.TrimSpace(pin) != pin {
		return fmt.Errorf("%s no debe empezar ni terminar con espacios", oauthPINEnvName)
	}
	return nil
}

func dotenvPIN(path string) (string, bool, error) {
	meta, err := os.Lstat(path)
	if errors.Is(err, os.ErrNotExist) {
		return "", false, nil
	}
	if err != nil {
		return "", false, fmt.Errorf("leer configuración privada .env: %w", err)
	}
	if !meta.Mode().IsRegular() {
		return "", false, fmt.Errorf("el archivo .env debe ser regular, no un enlace simbólico")
	}
	if runtime.GOOS != "windows" && meta.Mode().Perm()&0077 != 0 {
		return "", false, fmt.Errorf("el archivo .env no es privado; aplica chmod 600 %s", path)
	}
	if meta.Size() > 64*1024 {
		return "", false, fmt.Errorf("el archivo .env excede 64 KiB")
	}
	file, err := os.Open(path)
	if err != nil {
		return "", false, fmt.Errorf("abrir .env: %w", err)
	}
	defer file.Close()
	scanner := bufio.NewScanner(file)
	scanner.Buffer(make([]byte, 4*1024), 64*1024)
	var found bool
	var value string
	for scanner.Scan() {
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "#") {
			continue
		}
		line = strings.TrimPrefix(line, "export ")
		k, raw, ok := strings.Cut(line, "=")
		if !ok || strings.TrimSpace(k) != oauthPINEnvName {
			continue
		}
		if found {
			return "", false, fmt.Errorf("clave %s duplicada en .env", oauthPINEnvName)
		}
		found = true
		raw = strings.TrimSpace(raw)
		if len(raw) >= 2 && (raw[0] == '\'' || raw[0] == '"') && raw[len(raw)-1] == raw[0] {
			raw = raw[1 : len(raw)-1]
		} else {
			// Inline comments need a separating space; # inside a PIN is valid.
			if before, _, exists := strings.Cut(raw, " #"); exists {
				raw = strings.TrimSpace(before)
			}
		}
		value = raw
	}
	if err := scanner.Err(); err != nil {
		return "", false, fmt.Errorf("analizar .env: %w", err)
	}
	return value, found, nil
}
