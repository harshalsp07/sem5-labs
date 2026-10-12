import pty
import os
import time
import subprocess
import sys

def test_shell():
    master, slave = pty.openpty()
    proc = subprocess.Popen(
        ['./shell'],
        stdin=slave,
        stdout=slave,
        stderr=slave,
        cwd='/home/harshal/course/os/Lab-2',
        close_fds=True,
        text=True
    )
    os.close(slave)

    def read_until_prompt(timeout=5):
        buffer = ""
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                data = os.read(master, 1024).decode('utf-8')
                if data:
                    buffer += data
                    if "shell> " in buffer:
                        return buffer
            except OSError:
                break
            time.sleep(0.05)
        return buffer

    def send_cmd(cmd):
        os.write(master, (cmd + "\n").encode('utf-8'))

    print("--- Test 1: Waiting for initial prompt ---")
    out = read_until_prompt()
    print("Initial output:", repr(out))
    assert "shell> " in out, "Initial prompt not found"

    print("\n--- Test 2: Simple Foreground Command ---")
    send_cmd("echo Hello Foreground")
    out = read_until_prompt()
    print("FG Output:", repr(out))
    assert "Hello Foreground" in out

    print("\n--- Test 3: Background Command Immediate Return ---")
    t0 = time.time()
    send_cmd("sleep 2 &")
    out = read_until_prompt()
    t1 = time.time()
    elapsed = t1 - t0
    print(f"BG Output (elapsed {elapsed:.2f}s):", repr(out))
    assert elapsed < 0.5, f"Background command blocked for {elapsed:.2f}s instead of returning immediately!"

    print("\n--- Test 4: Background process finished check ---")
    print("Waiting 2.5s for sleep 2 & to finish...")
    time.sleep(2.5)
    send_cmd("echo Check BG")
    out = read_until_prompt()
    print("Check output:", repr(out))
    assert "Shell: Background process finished" in out, "Background process finished message missing!"

    print("\n--- Test 5: Background finished DURING Foreground execution ---")
    # Start 1-second background job
    send_cmd("sleep 1 &")
    read_until_prompt()
    
    # Start 3-second foreground job immediately after
    t0 = time.time()
    send_cmd("sleep 3")
    out = read_until_prompt(timeout=5)
    t1 = time.time()
    elapsed = t1 - t0
    print(f"Combined FG/BG output (elapsed {elapsed:.2f}s):", repr(out))
    assert elapsed >= 2.8, f"Shell returned too early ({elapsed:.2f}s)! Expected foreground process to take ~3s."
    assert "Shell: Background process finished" in out, "Background process reaped message missing after FG finished!"

    print("\n--- Test 5b: Multiple Background processes finishing ---")
    send_cmd("sleep 1 &")
    read_until_prompt()
    send_cmd("sleep 1 &")
    read_until_prompt()
    send_cmd("sleep 1 &")
    read_until_prompt()
    time.sleep(1.5)
    send_cmd("echo Check Multi BG")
    out = read_until_prompt()
    print("Multi BG output:", repr(out))
    assert out.count("Shell: Background process finished") == 3, f"Expected 3 finished messages, got {out.count('Shell: Background process finished')}"

    print("\n--- Test 6: Builtin exit ---")
    send_cmd("exit")
    time.sleep(0.5)
    print("Shell process poll return status:", proc.poll())

    os.close(master)
    print("\nALL TESTS PASSED SUCCESSFULLY!")

if __name__ == '__main__':
    test_shell()
