Aaron Lewis CS4600 P1

1. Place both AES128Encrypt.cpp and aes128_constants.h in the same directory
2. In the source code directory, compile AES128Encrypt.cpp using command:
	g++ -o AES128Encrypt AES128Encrypt.cpp
3. Run AES128Encrypt:
	./AES128Encrypt <full path to file to encrypt>
4. The destination of the encrypted file will be printed. You will be prompted to enter the hex key. Enter 32 hex values. All other values will be ignored and you cannot continue until you enter 32 valid hex values.

5. After entering the key, the file will be encrypted and placed in the same directory.