Import("env") # type: ignore
import subprocess

def run_command(command, new_console=False, minimized=False):
    if new_console:
        # Build the start command
        start_cmd = 'start "{}"'.format("")  # pusty tytuł
        if minimized:
            start_cmd += " /MIN"
        start_cmd += f' cmd /k "{command}"'
        subprocess.Popen(start_cmd, shell=True)
    else:
        result = subprocess.run(command, shell=True)
        if result.returncode != 0:
            raise Exception(f"Command failed: {command}")

def kill_pio_remote_agent():
    """
    Kill any running PlatformIO Remote Agent process.
    """
    try:        
        print(f"0")
        result = subprocess.run('wmic process where "name=\'python.exe\'" get ProcessId,CommandLine', 
                                shell=True, capture_output=True, text=True)
        print(f"1")
        for line in result.stdout.splitlines():
            if "remote agent start" in line.lower():
                print(f"2")
                parts = line.strip().split()
                pid = int(parts[-1]) 
                subprocess.run(f"taskkill /PID {pid} /F", shell=True)
                print(f"Killed PlatformIO Remote Agent PID {pid}")
    except Exception as e:
        print(f"Could not kill PlatformIO Remote Agent: {e}")

def mega_test_audio(source, target, env):
    run_command("echo test audio v2")
    
    # 0. kill if exists
    print(">>> kill cmd pio_remote_agent")
    kill_pio_remote_agent()

    # 1. Build + Upload
    print(">>> Building & Uploading firmware...")
    run_command("platformio run --target upload --environment esp32doit-devkit-v1 --upload-port COM5")

    # 2. Run audio test
    print(">>> Running audio test...")
    run_command("pio test -e esp32doit-devkit-v1 -f test_audio --without-testing")

    # 3. Start PlatformIO Remote Agent (new console)
    print(">>> Starting PlatformIO Remote Agent in new console...")
    run_command("platformio remote agent start", new_console=True)

    # 4. Start Remote Device Monitor (new console)
    print(">>> Starting Remote Device Monitor in new console...")
    run_command("platformio remote device monitor")

def remote_monitor(source, target, env):
    run_command("echo Remote Monitor")
    
    # 0. kill if exists
    print(">>> kill cmd pio_remote_agent")
    kill_pio_remote_agent()

    # 1. Build + Upload
    print(">>> Building & Uploading firmware...")
    run_command("platformio run --target upload --environment esp32doit-devkit-v1 --upload-port COM5")
    
    # 3. Start PlatformIO Remote Agent (new console)
    print(">>> Starting PlatformIO Remote Agent in new console...")
    run_command("platformio remote agent start", new_console=True)

    # 4. Start Remote Device Monitor (new console)
    print(">>> Starting Remote Device Monitor in new console...")
    run_command("platformio remote device monitor")


env.AddCustomTarget( # type: ignore
    name="mega_test_audio",
    dependencies=None,
    actions=[mega_test_audio],
    title="Mega Test Audio",
    description="Build+upload, run test, start remote agent and monitor"
)


env.AddCustomTarget( # type: ignore
    name="remote_monitor",
    dependencies=None,
    actions=[remote_monitor],
    title="Remote Monitor",
    description="Build+upload, start remote agent and monitor"
)