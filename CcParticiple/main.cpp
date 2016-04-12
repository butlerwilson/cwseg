#include "CcParticiple.h"

int main(int argc, char **argv)
{
	int res = 0;
	int wlen = 4;

	if (argc != 3) {
		std::cout << "Usage:CcParticiple infile outfile" << std::endl;
		system("pause");

		return 0;
	}

	CcParticiple *ccp = new CcParticiple(argv[1], argv[2], wlen);
	res = ccp->CcP_init_filestream();
	if (res) return 1;

	ccp->CcP_init_statistics();
	ccp->CcP_word_statistics();
//	ccp->CcP_word_probability();

	std::cout << "analyzing the words..." << std::endl;
	ccp->CcP_word_analyze();
	ccp->CcP_word_print();
	ccp->CcP_word_wback();

	ccp->CcP_end_filestream();
	std::cout << "Run time:" << clock() / CLOCKS_PER_SEC << "s." <<std::endl;
	system("pause");

	return 0;
}