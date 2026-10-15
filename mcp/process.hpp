#pragma once
#include "common.hpp"
#include <cstdio>
#include <thread>
#include <atomic>
#include <functional>
#ifdef _WIN32
#include <io.h>
#else
#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
extern char** environ;
#endif

namespace gptframes::mcp {
struct ProcessResult {
    int exit_code = -1;
    bool timed_out = false;
    bool cancelled = false;
    std::string output;
};
inline void append_bounded(std::string& output, const char* data, std::size_t length) {
    constexpr std::size_t capacity = 65536;
    if (length >= capacity) { output.assign(data + length - capacity, capacity); return; }
    if (output.size() + length > capacity) output.erase(0, output.size() + length - capacity);
    output.append(data, length);
}
#ifdef _WIN32
inline std::wstring utf16(const std::string& s) {
    if (s.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), static_cast<int>(s.size()), nullptr, 0);
    if (!size) throw std::runtime_error("Texto UTF-8 invalido");
    std::wstring out(static_cast<std::size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s.data(), static_cast<int>(s.size()), out.data(), size);
    return out;
}
inline std::wstring windows_argument(const std::string& s) {
    const auto value = utf16(s);
    std::wstring result = L"\"";
    std::size_t slashes = 0;
    for (auto c : value) {
        if (c == L'\\') { ++slashes; continue; }
        if (c == L'"') result.append(slashes * 2 + 1, L'\\');
        else result.append(slashes, L'\\');
        slashes = 0;
        result += c;
    }
    result.append(slashes * 2, L'\\');
    return result + L"\"";
}
#endif
inline ProcessResult run_process(const std::vector<std::string>& arguments, int timeout_seconds = 75,
                                 const std::string& input = {},
                                 const std::function<void(const std::string&)>& on_output = {},
                                 const std::atomic_bool* cancel = nullptr) {
    if (arguments.empty() || arguments.front().empty() || timeout_seconds < 1)
        throw std::invalid_argument("Proceso no valido");
    for (const auto& a : arguments) if (a.find('\0') != std::string::npos)
        throw std::invalid_argument("Argumento con NUL");
    struct CloseFile { void operator()(FILE* file) const { if (file) std::fclose(file); } };
    std::unique_ptr<FILE, CloseFile> in(std::tmpfile());
    if (!in || std::fwrite(input.data(), 1, input.size(), in.get()) != input.size() || std::fflush(in.get()))
        throw std::runtime_error("No se pudo preparar stdin");
    std::rewind(in.get());
    ProcessResult result;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds);
#ifdef _WIN32
    struct Handle {
        HANDLE value = nullptr;
        ~Handle() { if (value && value != INVALID_HANDLE_VALUE) CloseHandle(value); }
    };
    SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    Handle read_end, write_end, input_handle, job;
    if (!CreatePipe(&read_end.value, &write_end.value, &security, 0) ||
        !SetHandleInformation(read_end.value, HANDLE_FLAG_INHERIT, 0))
        throw std::runtime_error("No se pudo crear canal de proceso");
    if (!DuplicateHandle(GetCurrentProcess(), reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(in.get()))),
                         GetCurrentProcess(), &input_handle.value, 0, TRUE, DUPLICATE_SAME_ACCESS))
        throw std::runtime_error("No se pudo preparar stdin nativo");
    std::wstring command;
    for (const auto& a : arguments) { if (!command.empty()) command += L' '; command += windows_argument(a); }
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
    startup.hStdInput = input_handle.value;
    startup.hStdOutput = write_end.value; startup.hStdError = write_end.value;
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, TRUE,
                        CREATE_NO_WINDOW | CREATE_SUSPENDED, nullptr, nullptr, &startup, &process))
        throw std::runtime_error("No se pudo iniciar ejecutable");
    Handle process_handle, thread_handle;
    process_handle.value = process.hProcess; thread_handle.value = process.hThread;
    job.value = CreateJobObjectW(nullptr, nullptr);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (!job.value || !SetInformationJobObject(job.value, JobObjectExtendedLimitInformation, &limits, sizeof(limits)) ||
        !AssignProcessToJobObject(job.value, process.hProcess)) {
        TerminateProcess(process.hProcess, 127);
        WaitForSingleObject(process.hProcess, INFINITE);
        throw std::runtime_error("No se pudo aislar el grupo de procesos");
    }
    ResumeThread(process.hThread);
    CloseHandle(write_end.value); write_end.value = nullptr;
    bool finished = false;
    while (true) {
        DWORD available = 0;
        if (PeekNamedPipe(read_end.value, nullptr, 0, nullptr, &available, nullptr) && available) {
            char block[4096]; DWORD received = 0;
            if (ReadFile(read_end.value, block, std::min<DWORD>(sizeof(block), available), &received, nullptr))
                { append_bounded(result.output, block, received);
                  if(on_output)on_output(std::string(block,received)); }
        } else {
            if (!finished) finished = WaitForSingleObject(process.hProcess, 0) == WAIT_OBJECT_0;
            if (finished) break;
        }
        if ((cancel && cancel->load()) || std::chrono::steady_clock::now() >= deadline) {
            result.cancelled = cancel && cancel->load();
            result.timed_out = !result.cancelled;
            TerminateJobObject(job.value, result.cancelled ? 130 : 124);
            WaitForSingleObject(process.hProcess, INFINITE);
            break;
        }
        if (!available) std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    DWORD code = 0;
    GetExitCodeProcess(process.hProcess, &code);
    result.exit_code = result.cancelled ? 130 : result.timed_out ? 124 : static_cast<int>(code);
#else
    struct Descriptor {
        int value = -1;
        ~Descriptor() { if (value >= 0) close(value); }
    };
    int pipe_ends[2];
#ifdef __linux__
    if (pipe2(pipe_ends, O_CLOEXEC)) throw std::runtime_error("No se pudo crear pipe");
#else
    if (pipe(pipe_ends)) throw std::runtime_error("No se pudo crear pipe");
    fcntl(pipe_ends[0], F_SETFD, FD_CLOEXEC);
    fcntl(pipe_ends[1], F_SETFD, FD_CLOEXEC);
#endif
    Descriptor read_end{pipe_ends[0]}, write_end{pipe_ends[1]};
    fcntl(fileno(in.get()), F_SETFD, FD_CLOEXEC);
    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attributes;
    posix_spawn_file_actions_init(&actions);
    posix_spawnattr_init(&attributes);
    posix_spawn_file_actions_adddup2(&actions, fileno(in.get()), STDIN_FILENO);
    posix_spawn_file_actions_adddup2(&actions, write_end.value, STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, write_end.value, STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, read_end.value);
    posix_spawn_file_actions_addclose(&actions, write_end.value);
    posix_spawnattr_setflags(&attributes, POSIX_SPAWN_SETPGROUP);
    posix_spawnattr_setpgroup(&attributes, 0);
    std::vector<char*> argv;
    for (const auto& a : arguments) argv.push_back(const_cast<char*>(a.c_str()));
    argv.push_back(nullptr);
    pid_t child = -1;
    const int error = posix_spawnp(&child, argv[0], &actions, &attributes, argv.data(), environ);
    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attributes);
    if (error) throw std::runtime_error("No se pudo iniciar ejecutable: " + std::to_string(error));
    close(write_end.value); write_end.value = -1;
    fcntl(read_end.value, F_SETFL, fcntl(read_end.value, F_GETFL, 0) | O_NONBLOCK);
    bool finished = false, eof = false;
    int status = 0;
    while (!finished || !eof) {
        char block[4096];
        ssize_t count;
        // Yield to deadline/wait checks even if a child continuously fills stdout.
        count = -1;
        for (int blocks = 0; blocks < 16; ++blocks) {
            count = read(read_end.value, block, sizeof(block));
            if (count <= 0) break;
            { append_bounded(result.output, block, static_cast<std::size_t>(count));
              if(on_output)on_output(std::string(block,static_cast<std::size_t>(count))); }
        }
        if (count == 0) eof = true;
        if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) eof = true;
        if (!finished) {
            const auto waited = waitpid(child, &status, WNOHANG);
            if (waited == child) finished = true;
            else if (waited < 0 && errno != EINTR) {
                kill(-child, SIGKILL);
                throw std::runtime_error("No se pudo esperar al ejecutable");
            }
        }
        if (finished && eof) break;
        if ((cancel && cancel->load()) || std::chrono::steady_clock::now() >= deadline) {
            result.cancelled = cancel && cancel->load();
            result.timed_out = !result.cancelled;
            kill(-child, SIGKILL);
            if (!finished) while (waitpid(child, &status, 0) < 0 && errno == EINTR) {}
            break;
        }
        pollfd watched{read_end.value, POLLIN, 0};
        poll(&watched, 1, 10);
    }
    // Kill any descendants that outlived the requested program.
    kill(-child, SIGKILL);
    result.exit_code = result.cancelled ? 130 : result.timed_out ? 124 :
        WIFEXITED(status) ? WEXITSTATUS(status) : WIFSIGNALED(status) ? 128 + WTERMSIG(status) : -1;
#endif
    return result;
}
} // namespace gptframes::mcp
