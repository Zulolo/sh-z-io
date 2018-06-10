@echo off
set ROOT_PATH="G:\test_tool\modpoll\win32\"
IF "%1"=="query" (
	Echo Start query......
	%ROOT_PATH%modpoll.exe -1 -t 0 -r 1 -c 4 192.168.0.126
	%ROOT_PATH%modpoll.exe -1 -t 0 -r 33 -c 4 192.168.0.126
	%ROOT_PATH%modpoll.exe -1 -t 0 -r 97 -c 4 192.168.0.126
	%ROOT_PATH%modpoll.exe -1 -t 0 -r 129 -c 4 192.168.0.126
	%ROOT_PATH%modpoll.exe -1 -t 0 -r 161 -c 4 192.168.0.126
	%ROOT_PATH%modpoll.exe -1 -t 3:hex -r 40001 -c 8 192.168.0.126
) ELSE IF "%1"=="cnt_enable" (
	Echo Enable DI counter......
	
) ELSE IF "%1"=="clear_cnt" (
	Echo Clear DI counter......
) ELSE IF "%1"=="set_latch" (
	Echo Config latch edge......
) ELSE IF "%1"=="clear_latch" (
	Echo Clear latch status......
) ELSE (
	Echo check input parameter
)

