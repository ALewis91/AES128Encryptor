//
//  main.cpp
//  AES128Encrypt
//
//  Created by Aaron Lewis on 3/11/20.
//  Copyright © 2020 Aaron Lewis. All rights reserved.
//
#include <iostream>
#include <string>
#include "aes128_constants.h"

std::string get_dst_filepath(std::string filepath);
std::string get_key();
void create_key_bytes(unsigned char dst[], std::string keyString);
void get_key_schedule(unsigned char key[], unsigned char key_schedule[11][16]);
int hex_to_dec(char c);
void g_function(unsigned char key[], unsigned char result[], int round_number);
void XOR_word(unsigned char src1[], int start_index1, unsigned char src2[], int start_index2, unsigned char dst[], int dst_start_index);
void key_add(unsigned char key[], unsigned char buffer[]);
void byte_substitution(unsigned char buffer[]);
void shift_rows(unsigned char buffer[]);
void mix_columns(unsigned char *input);


int main(int argc, const char * argv[])
{
    std::string dst_filepath, keyString;
    unsigned char key[16];
    unsigned char key_schedule[11][16];
    
    // Create destination file from input filepath and filename
    dst_filepath = get_dst_filepath(argv[1]);
    
    
    std::cout << "Dst filepath: " << dst_filepath << std::endl;
    
    // Obtain hex key from user
    std::cout << "Enter the encryption key in hex format: ";
    keyString = get_key();
    
    // Converts hex key from string to binary
    create_key_bytes(key, keyString);
    
    // Creates key schedule from key and stores in key_schedule
    get_key_schedule(key, key_schedule);
    
    // Open and store source and destination file descriptors
    FILE *src_file_ptr, *dst_file_ptr;
    src_file_ptr = fopen(argv[1], "rb");
    dst_file_ptr = fopen(dst_filepath.c_str(), "wb");
    
    // Buffer to store input bytes
    unsigned char f_buffer[16];
    size_t bytes_read = 0;
    
    // Read until end of file
    do
    {
        // Grab up to the next 16 bytes of the file
        bytes_read = fread(f_buffer, sizeof(unsigned char), 16, src_file_ptr);
        
        // If number of bytes is less than 16, pad the buffer
        if (bytes_read < 16)
        {
            for (int x = (int)bytes_read; x < 16; x++)
                f_buffer[x] = 16 - bytes_read;
        }
        
        key_add(key_schedule[0], f_buffer);
        
        // Repeat for 9 rounds
        for (int i = 1; i <= 9; i++)
        {
            byte_substitution(f_buffer);
            shift_rows(f_buffer);
            mix_columns(f_buffer);
            
            // Apply key addition using the ith key from the key schedule
            key_add(key_schedule[i], f_buffer);
        }
        
        byte_substitution(f_buffer);
        shift_rows(f_buffer);
        
        // Apply key addition using the last key from the key schedule
        key_add(key_schedule[10], f_buffer);
        
        // Write the buffer to the output file
        fwrite(f_buffer, 1, 16, dst_file_ptr);
        
    } while (bytes_read == 16);
    
    // Close the files
    fclose(src_file_ptr);
    fclose(dst_file_ptr);
     
    return 0;
}

std::string get_dst_filepath(std::string filepath)
{
    std::string dst_filepath;
    int endIndex = 0;
    for (int x = (int)filepath.length(); x >= 0; x--)
    {
        if (filepath[x] == '.')
        {
            endIndex = x;
            x = -1;
        }
    }
    dst_filepath = filepath.substr(0, endIndex).append(".enc");
    return dst_filepath;
}

std::string get_key()
{
    std::string tempKey = "";
    char c;
    while (tempKey.length() < 32)
    {
        std::cin.get(c);
        if (c == 'x' || c == 'X')
        {
            tempKey = "";
        }
        else if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == 'A' || c == 'a' || c == 'B' || c == 'b' || c == 'C' || c == 'c' || c == 'D' || c == 'd' || c == 'E' || c == 'e' || c == 'F' || c == 'f')
            tempKey += c;
    }
    return tempKey;
}

void create_key_bytes(unsigned char dst[], std::string keyString)
{
    for (int x = 0; x < keyString.length(); x++)
    {
        dst[x/2] <<= 4;
        dst[x/2] |= hex_to_dec(keyString[x]);
    }
}

int hex_to_dec(char c)
{
    if (c == '0')
        return 0;
    else if (c == '1')
        return 1;
    else if (c == '2')
        return 2;
    else if (c == '3')
        return 3;
    else if (c == '4')
        return 4;
    else if (c == '5')
        return 5;
    else if (c == '6')
        return 6;
    else if (c == '7')
        return 7;
    else if (c == '8')
        return 8;
    else if (c == '9')
        return 9;
    else if (c == 'a' || c == 'A')
        return 10;
    else if (c == 'b' || c == 'B')
        return 11;
    else if (c == 'c' || c == 'C')
        return 12;
    else if (c == 'd' || c == 'D')
        return 13;
    else if (c == 'e' || c == 'E')
        return 14;
    else if (c == 'f' || c == 'F')
        return 15;
    else
        return -1;
}

void get_key_schedule(unsigned char key[], unsigned char key_schedule[11][16])
{
    // Copy key for first key in the key schedule
    for (int x = 0; x < 16; x++)
        key_schedule[0][x] = key[x];
    
    //
    unsigned char g_function_result[4];
    for (int x = 0; x < 10; x++)
    {
        // Obtain result of g function and store it in g_function_result
        g_function(key_schedule[x], g_function_result, x+1);
        
        // 
        XOR_word(key_schedule[x], 0, g_function_result, 0, key_schedule[x+1], 0);
        XOR_word(key_schedule[x], 4, key_schedule[x+1], 0, key_schedule[x+1], 4);
        XOR_word(key_schedule[x], 8, key_schedule[x+1], 4, key_schedule[x+1], 8);
        XOR_word(key_schedule[x], 12, key_schedule[x+1], 8, key_schedule[x+1], 12);
    }
}

void g_function(unsigned char key[], unsigned char result[], int round_number)
{
    result[0] = (sbox[key[13]] ^ rcon[round_number]);
    result[1] = sbox[key[14]];
    result[2] = sbox[key[15]];
    result[3] = sbox[key[12]];
}

void XOR_word(unsigned char src1[], int start_index1, unsigned char src2[], int start_index2, unsigned char dst[], int dst_start_index)
{
    for (int x = 0; x < 4; x++)
    {
        dst[dst_start_index+x] = src1[x + start_index1] ^ src2[x + start_index2];
    }
}

void key_add(unsigned char key[], unsigned char buffer[])
{
    for (int x = 0; x < 16; x++)
        buffer[x] = key[x]^buffer[x];
}

void byte_substitution(unsigned char buffer[])
{
    for (int x = 0; x < 16; x++)
        buffer[x] = sbox[buffer[x]];
}

void shift_rows(unsigned char buffer[])
{
    unsigned char temp[16];
    for (int x = 0; x < 16; x++)
        temp[x] = buffer[x];
    buffer[1] = temp[5];
    buffer[2] = temp[10];
    buffer[3] = temp[15];
    buffer[5] = temp[9];
    buffer[6] = temp[14];
    buffer[7] = temp[3];
    buffer[9] = temp[13];
    buffer[10] = temp[2];
    buffer[11] = temp[7];
    buffer[13] = temp[1];
    buffer[14] = temp[6];
    buffer[15] = temp[11];
}

void mix_columns(unsigned char *input) {
    unsigned char tmp[16];
    int i;
    for (i = 0; i < 4; ++i) {
        tmp[(i << 2) + 0] = (unsigned char) (mul2[input[(i << 2) + 0]] ^ mul_3[input[(i << 2) + 1]] ^ input[(i << 2) + 2] ^ input[(i << 2) + 3]);
        tmp[(i << 2) + 1] = (unsigned char) (input[(i << 2) + 0] ^ mul2[input[(i << 2) + 1]] ^ mul_3[input[(i << 2) + 2]] ^ input[(i << 2) + 3]);
        tmp[(i << 2) + 2] = (unsigned char) (input[(i << 2) + 0] ^ input[(i << 2) + 1] ^ mul2[input[(i << 2) + 2]] ^ mul_3[input[(i << 2) + 3]]);
        tmp[(i << 2) + 3] = (unsigned char) (mul_3[input[(i << 2) + 0]] ^ input[(i << 2) + 1] ^ input[(i << 2) + 2] ^ mul2[input[(i << 2) + 3]]);
    }

    for (i = 0; i < 16; ++i)
        input[i] = tmp[i];
}
