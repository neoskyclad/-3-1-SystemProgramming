#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "20192800.h"

FILE *file;

//결과값을 각 배열에 저장하기 위한 인덱스 배열
int index_result[7] = { 0, };

int main()
{
	int bufsiz = 0;

	file = fopen("input", "r");

	//input의 버퍼 사이즈를 얻기 위한 while문
	while(!feof(file))
	{
		bufsiz++;
		fgetc(file);
	}

	//input 문자열을 담을 배열과 길이 선언
	int str_length = 0;
	char *str = (char*)malloc(bufsiz - 2);

	//파일 인덱스 초기화
	fseek(file, 0, SEEK_SET);

	//input 파일로부터 0과 1을 검사하고 str에 저장
	while(!feof(file))
	{
		char c = fgetc(file);
		if(c == '0' || c == '1')
			str[str_length++] = c;
	}

	//구분된 비트들을 각기 다른 자료형으로 출력하기 위한 결과값 배열 선언 
	char *result_sc = (char*)malloc(str_length / 8);
	char *result_ac = (char*)malloc(str_length / 8);
	unsigned char *result_uc = (unsigned char*)malloc(str_length / 8);
	int *result_si = (int*)malloc(str_length / 32);
	unsigned int *result_ui = (unsigned int*)malloc(str_length / 32);
	float *result_sf = (float*)malloc(str_length / 32);
	double *result_sd = (double*)malloc(str_length / 64);

	//8비트(=1바이트)를 원소로 하는 2차원 문자 배열
	char **buffer;

	//2차원 배열 동적 할당
	buffer = (char**)malloc(sizeof(char*) * str_length / 8);
	for(int i = 0; i < str_length / 8; i++)
		buffer[i] = (char*)malloc(sizeof(char) * 8);

	//1바이트, 4바이트, 8바이트 버퍼 선언
	uint8_t buffer_onebyte;
	uint32_t buffer_fourbyte;
	uint64_t buffer_eightbyte;
	int buffer_index = 0;

	//0번 인덱스부터 input 문자열의 끝까지 탐색하여 8비트씩 잘라서 buffer 배열에 입력하는 for문
	for(int i = 0; i < str_length; i++)
	{
		if((i + 1) % 8 == 0)
		{
			int small_index = 0;
			for (int j = i - 7; j <= i; j++)
				buffer[buffer_index][small_index++] = str[j];
			buffer_index++;
		}
	}
	buffer_index--;
	
	//저장된 buffer를 top 인덱스에서 0번 인덱스로 스택처럼 뒤에서부터 탐색하는 for문
	for(int i = buffer_index; i >= 0; i--)
	{
		//1바이트일 때
		if((buffer_index - i + 1) % 1 == 0)
		{
			//비트로 표기된 문자열을 정수로 변환해주는 strtol
			buffer_onebyte = (uint8_t)strtol(buffer[i], NULL, 2);

			//각각 signed_char, ASCII_codes, unsigned_char 배열에 저장
			signed_char(result_sc, buffer_onebyte);
			ASCII_codes(result_ac, buffer_onebyte);
			unsigned_char(result_uc, buffer_onebyte);
		}
		//4바이트일 때
		if((buffer_index - i + 1) % 4 == 0)
		{
			//32비트(4바이트)의 임시 버퍼 선언
			char buffer_temp[32] = ""; 

			//strcat으로 비트로 표기된 문자열 이어 붙이기
			for(int j = i + 3; j >= i; j--)
				strcat(buffer_temp, buffer[j]);

			//문자열을 4바이트 버퍼로 변환하여 할당
			buffer_fourbyte = (uint32_t)strtol(buffer_temp, NULL, 2);

			//signed_int, unsigned_int, sigend_float 배열에 저장
			signed_int(result_si, buffer_fourbyte);
			unsigned_int(result_ui, buffer_fourbyte);
			signed_float(result_sf, (int32_t)buffer_fourbyte);
		}
		//8바이트일 때
		if((buffer_index - i + 1) % 8 == 0)
		{
			//64비트(8바이트)의 임시 버퍼 선언
			char buffer_temp[64] = "";
			for(int j = i + 7; j >= i; j--)
				strcat(buffer_temp, buffer[j]);

			//문자열을 8바이트 버퍼로 변환하여 할당
			buffer_eightbyte = (uint64_t)strtol(buffer_temp, NULL, 2);

			//signed_double 배열에 저장
			signed_double(result_sd, buffer_eightbyte);
		}
	}

	//버퍼 사이즈와 input 문자열 출력
	printf("BUFSIZ : %d\n\n", bufsiz);
	printf("input : %s\n\n", str);

	//저장된 결과 전부 출력
	printf("1. signed char : ");
	for(int i = 0; i < index_result[0]; i++)
		printf("%d ", result_sc[i]);
	printf("\n2. ASCII codes : ");
	for(int i = 0; i < index_result[1]; i++)
		printf("%c ", result_ac[i]);
	printf("\n3. unsigned char : ");
	for(int i = 0; i < index_result[2]; i++)
		printf("%ud ", result_uc[i]);
	printf("\n4. signed int : ");
	for(int i = 0; i < index_result[3]; i++)
		printf("%d ", (int32_t)result_si[i]);
	printf("\n5. unsigned int : ");
	for(int i = 0; i < index_result[4]; i++)
		printf("%ud ", (uint32_t)result_ui[i]);
	printf("\n6. signed float : ");
	for(int i = 0; i < index_result[5]; i++)
		printf("%.4f ", (float)result_sf[i]);
	printf("\n7. signed double : ");
	for(int i = 0; i < index_result[6]; i++)
		printf("%.4f ", (double)result_sd[i]);
	printf("\n");

	fclose(file);

	return 0;
}

//각 자료형에 알맞은 형변환과 함께 결과값 배열에 버퍼를 저장
void signed_char(char *str, uint8_t buffer)
{
	str[index_result[0]++] = (int8_t)buffer;

}

void ASCII_codes(char *str, uint8_t buffer)
{
	//만약 ASCII code 값을 벗어나면 .으로 저장
	if (buffer < 0 || buffer > 127)
		str[index_result[1]++] = '.';
	else
		str[index_result[1]++] = buffer;
}

void unsigned_char(unsigned char *str, uint8_t buffer)
{
	str[index_result[2]++] = buffer;
}

void signed_int(int *str, uint32_t buffer)
{
	str[index_result[3]++] = (int32_t)buffer;
}

void unsigned_int(unsigned int *str, uint32_t buffer)
{
	str[index_result[4]++] = (uint32_t)buffer;
}

void signed_float(float *str, int32_t buffer)
{
	str[index_result[5]++] = (float)buffer;
}

void signed_double(double *str, int64_t buffer)
{
	str[index_result[6]++] = (double)buffer;
}
