#pragma once

#include <map>
#include <fstream>
#include <ios>
#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>

#define WORD_SIZE	2
#define DWORD_SIZE	4
#define	DDWORD_SIZE	8

#define	WORD_INDEX	0
#define WORD_COUNT	4


typedef unsigned char uchar_t;

typedef std::string::iterator str_it;
typedef std::vector<std::string>::iterator vec_it;

typedef std::map<std::string, double>::iterator map_it;
typedef std::map<std::string, double>::reverse_iterator map_rit;

//#define DEBUG

//�ı�map��Ĭ���������
struct compare {
	bool operator()(const std::string str1, const std::string str2)
	{
		return str1.size() < str2.size();
	}
};

class CcParticiple {
public:
	CcParticiple(std::string in, std::string out, size_t wordlen = 2);
	int CcP_init_filestream();
	int CcP_end_filestream();

	int CcP_init_statistics();

	int CcP_word_statistics();
	int CcP_word_probability();

	bool CcP_word_erase();
	void CcP_word_analyze();

	void CcP_source_print();
	void CcP_word_print();
	void CcP_word_wback();

private:
	std::string infile;
	std::string outfile;

	std::ifstream in;
	std::ofstream out;

	size_t wlen;					//�ִʴ���󳤶�
	size_t words_length;			//�ʵĳ��ȣ����ֽڼ���

	std::string source_string;
	std::vector<std::string> source;

	//std::map<std::string, int> word_count;
	//ʹ��double���͵�һ�δ洢Ƶ�ȣ��ڶ��δ洢���ָ���
	std::map<std::string, double> word_count;
	double word_type;				//���³���
	double threshold;				//�����������ֵ��������Ϊ��һ����

	std::vector<std::string> punctuation;	//�洢������

	bool CcP_word_map_find(map_it &tmpit, std::string second, size_t &cnt);
	int  CcP_word_string_find(std::string substr, std::string word);
};
