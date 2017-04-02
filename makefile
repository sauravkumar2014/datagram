all:	echo client

echo:
	gcc echo.c -o echo.out
client:
	gcc client.c -o client.out

