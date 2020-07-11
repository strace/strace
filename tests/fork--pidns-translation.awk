/fork/ {
        match($0, "([0-9]+) in strace\x27s PID NS", a);
        if (a[1])
                fork_pid = a[1]
}

/exited with 0/ {
        if (!exit_pid)
                exit_pid = $1
}

END {
        if (!fork_pid || !exit_pid || fork_pid != exit_pid)
                exit 1
}
