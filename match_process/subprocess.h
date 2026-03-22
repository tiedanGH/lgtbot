#pragma once

#include <cstdio>
#include <string>
#include <vector>

// Spawns a child process with stdin/stdout redirected to pipes (parent side: write to child stdin, read child stdout).
class Subprocess
{
  public:
    Subprocess(std::vector<std::string> argv, std::string& error_out);
    ~Subprocess();

    Subprocess(const Subprocess&) = delete;
    Subprocess& operator=(const Subprocess&) = delete;

    [[nodiscard]] bool ok() const { return ok_; }

    [[nodiscard]] FILE* child_stdin() const { return child_stdin_; }
    [[nodiscard]] FILE* child_stdout() const { return child_stdout_; }

    void request_stop();
    void wait_exit();

  private:
    bool ok_{false};
    FILE* child_stdin_{nullptr};
    FILE* child_stdout_{nullptr};
#ifdef _WIN32
    void* process_handle_{nullptr};
#else
    int pid_{-1};
#endif
};
