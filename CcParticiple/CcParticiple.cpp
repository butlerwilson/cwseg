#include "CcParticiple.h"

CcParticiple::CcParticiple(std::string in, std::string out, size_t wordslen)
{
	infile = in;
	outfile = out;

	word_type = 0.0;
	threshold = 0.0;
	wlen = wordslen;
	words_length = wordslen * 2;

	//添加标点符号，读取文章时过滤掉标点符号
	punctuation.push_back("，");
	punctuation.push_back("。");
	punctuation.push_back("	");
	punctuation.push_back("-");
	punctuation.push_back("“");
	punctuation.push_back("”");
	punctuation.push_back("：");
	punctuation.push_back("！");
	punctuation.push_back("？");
	punctuation.push_back("（");
	punctuation.push_back("）");
	punctuation.push_back("《");
	punctuation.push_back("》");
	punctuation.push_back("{");
	punctuation.push_back("}");
	punctuation.push_back(" ");
	punctuation.push_back("；");
	punctuation.push_back("…");
	punctuation.push_back("\n\n");

}

int CcParticiple::CcP_init_filestream()
{
	in.open(infile, std::ios::in);
	out.open(outfile, std::ios::out);

	if (!in) {
		std::cerr << "Fatal Error:file " << infile.c_str() << "not exist!" << std::endl;
		return -1;
	}

	if (!out) {
		std::cerr << "Warning:file " << outfile.c_str() << "can't create!" << std::endl;
		return -1;
	}

	return 0;
}

int CcParticiple::CcP_end_filestream()
{
	in.close();
	out.close();

	return 0;
}

int CcParticiple::CcP_init_statistics()
{
	char buf[DWORD_SIZE];
	std::string tmp;
	source_string = "";

	while (!in.eof()) {
		memset(buf, '\0', DWORD_SIZE);
		in.read(buf, WORD_SIZE);
		tmp = buf;
		source.push_back(tmp);
		source_string += tmp;
	}

	return 0;
}

int CcParticiple::CcP_word_statistics()
{
	std::string tmp;

	for (int ii = 0; ii < wlen; ++ii) {
		for (vec_it it = source.begin(); it != source.end(); ++it) {
			tmp = "";
			for (int j = 0; j <= ii && (it + j) < source.end(); ++j) {
				if (!(find(punctuation.begin(), punctuation.end(), *(it + j)) == punctuation.end()))
					break;
				tmp += *(it + j);
			}
			if ((int)tmp.size() < (2 * (ii + 1)))
				continue;		//遇见标点且不够ii词长度则不添加，防止频度加一
			++word_count[tmp];
#ifdef DEBUG
			std::cout << "CcP_word_statistics-组建词：" << tmp.c_str() << std::endl;
#endif // DEBUG

		}
	}
	//删除所有单个字和删除三个字以上的频度为一的词
	CcP_word_erase();
	for (map_it it = word_count.begin(); it != word_count.end(); ++it)
		word_type += it->second;
	if (word_type)
		threshold = 1.0 / word_type;

	return 0;
}

int CcParticiple::CcP_word_probability()
{
	for (map_it it = word_count.begin(); it != word_count.end(); ++it) {
		it->second = it->second / word_type;
	}
	threshold = 1.0 / word_type;

	return 0;
}

//查找当前词的第二个字是另一个词的第一个字，并且返回那个词的迭代器
bool CcParticiple::CcP_word_map_find(map_it &tmpit, std::string second, size_t &cnt)
{
	for (map_it it = tmpit; it != word_count.end(); ++it) {
		std::string tmp(it->first, 0, WORD_SIZE);
		cnt++;
		if (tmp.compare(second) == 0) {
			tmpit = it;
			return true;
		}
	}
	return false;
}

/*从source_string中查找substr,查看substr后边的字是否是word，此时的组合有三种情况。
  1.substr后没有找到字word					返回值：0  表示是两个词
  2.substr后找到了字word							1  证明是同一个词组，按照左侧优先原则
  3.substr后找到字word，但是有时没有字word			2  说明是两个词，分开存储，分词的时候
  按照substr后是否有字，如果有则按substr分词，否则按第二个词分词
 */
int CcParticiple::CcP_word_string_find(std::string substr, std::string word)
{
	int flag = 0;
	std::string::size_type pos;
	std::string string = substr + word;

	pos = source_string.find(string, 0);
	if (pos == std::string::npos) {
		flag = 0;
	} else {
		std::string fisrt(substr, 0, WORD_SIZE);
		std::string second(substr, 2);
		std::string next_words = second + word;
		pos = 0;
		do {
			pos = source_string.find(next_words, pos * 2);
			if (pos != std::string::npos) {
				//if (pos - 2 < 0) {	此时的比较是错误的。将无符号和有符号比较！
				if (pos < 2) {			//文章的第一个词特殊处理
					flag |= 2;
					break;				//这绝对是另外一个词
				} else {
					std::string str(source_string, pos - 2, WORD_SIZE);
					if (str.compare(fisrt) == 0) {
						flag |= 1;
					} else {
						flag |= 2;
						break;
					}
				}
			} else {
				break;
			}
		} while (true);
		
	}
	return flag;
}

bool CcParticiple::CcP_word_erase()
{
	/*
	for (map_it it = word_count.begin(); it != word_count.end(); ++it) {
		if (it->second <= 1) {
			word_count.erase(it);	//当第一次执行到这时，it的值变为end。下一次的迭代会出现错误
			//erase的实现版本和c++ primer 4th不一样
		}
	}
	*/
	int size = DWORD_SIZE;
	for (map_it it = word_count.begin(); it != word_count.end();) {
		size = it->first.size();
		if (size == WORD_SIZE || (/*size != DWORD_SIZE && */it->second <= 1)) {
#ifdef DEBUG
			std::cout << "CcP_word_erase-删除词：" << it->first.c_str() << std::endl;
#endif // DEBUG
			word_count.erase(it++);
		} else {
			++it;
		}
	}
	return true;
}

void CcParticiple::CcP_word_analyze()
{
	/*我们针对汉字分词，我们规定分词规则是这样的：
		1.对所有的一个字的词我们都当作是一个词，我们不做分析
		2.针对两个字词，我们按照频率大小和最左优先原则规定他是一个词。
		3.针对三个往上的词，我们只做频度大于一的词的分词，其余按照两个字组成的词那样处理。
	*/

	/*如何判定当前词是否是一个适当的词：
	  例如 BC--3		AB--3
	  针对两个字组成的词，如果当前词（BC）的第一个字(B)是另一个词(AB)的最后一个字(B)
	  并且当前（BC）概率和另一个词(AB)的概率相同并且(AB)的下一个字是(C)，那么我们将
	  (AB)作为一个更合适的词，如果(AB)和(BC)概率不同并且(AB)下一个字是(C),那么谁的概率大，
	  谁将是更合适的词。否则，如果(AB)的下一个字不是(C)，那么他们是两个分开的词。（优先匹配原则）
	  考虑一个特例：
	  ABC，ABC，BC，BC，BC，BC。
	  此时违反我们上述的算法规则，那我们就需要遍历《BC》得前一个字看是否是《A》，如果不是，
	  则按两个词组计算，如果是则按照上述算法分词。

	  经过分析，我们可以同时处理两个字和两个字往上组成的词。
	  如果要求最多 四个字 可以组成一个词，那么有：
	  比如现在有这样的一个例子：原文出现四个 ABCD,ABCD,ABCD,ABCD。
	  经过初次的统计之后，我们发现AB,BC,CD,ABC,BCD,ABCD都各自出现了四次。
	  现在，我们知道我们需要的词是ABCD而不是其他的组合，此时其他的组合都是ABCD的子集。
	  那么，此时我们就应该按照ABCD的频度和AB,BC,CD,ABC,BCD的频度比较。
	  如果子集的频度小于等于ABCD的频度，则删除其子集，否则按照不同词处理。
	  如果有原文：ABCDE, ABCDE, ABCDE, ABCDE
	  此时统计出来的结果将是：
		ABCD 4		BCDE 4	另外加上其子集。
	  按照两个字的处理方式，先读到《ABCD》该词组，取其从第二个字开始往后组词.
	  即《BCD》，《CD》，《D》。
	  如果是《BCD》则将判断《BCD》，《BCDE》是否是合适的词，按频度判断。
	  如果是《CD》则将判断《CD》，《CDE》，《CDEF》是否是合适的词。
	  如果是《D》则判断《DE》，《DEF》，《DEFG》是否是合适的词语。
	*/
	size_t rcnt = 1;
	for (map_rit it = word_count.rbegin(); it != word_count.rend(); ++it, rcnt++) {
		size_t curr_wsize = it->first.size();
		std::string curr_words = it->first;
#ifdef DEBUG
		std::cout << "current word:" << curr_words.c_str() << std::endl;
#endif // DEBUG
		for (size_t ii = WORD_INDEX; ii < curr_wsize; ii += WORD_SIZE) {
			for (size_t cn = WORD_COUNT; cn <= curr_wsize - ii; cn += WORD_SIZE) {
				size_t cnt = 0;
				int erase_flag = 0;
				map_it tmpit = word_count.begin();
				//一次最少复制两个字，提高效率
				std::string subwords(curr_words, ii, cn);
				size_t sub_wsize = subwords.size();
#ifdef DEBUG
				std::cout << subwords.c_str() << "\t";
#endif // DEBUG
				std::string prefix_words(subwords, 0, sub_wsize - WORD_SIZE);
				std::string suffix_words(subwords, sub_wsize - WORD_SIZE, WORD_SIZE);
				while (CcP_word_map_find(tmpit, suffix_words, cnt)) {
					if (tmpit != word_count.end()) {
						std::string next_suffix_words(tmpit->first, 2, tmpit->first.size() - WORD_SIZE);
						int res = CcP_word_string_find(subwords, next_suffix_words);
						if (res == 1) {
							if (ii > WORD_INDEX && (sub_wsize == curr_wsize - WORD_SIZE)) {
								if (word_count[curr_words] < tmpit->second)
									erase_flag = 1;
							}
							if (word_count[curr_words] >= tmpit->second) {
#ifdef DEBUG
								std::cout << "delete:" << tmpit->first.c_str() << std::endl;
#endif // DEBUG
								if (cnt >= word_count.size() - rcnt) {
#ifdef DEBUG
									std::cout << "reset the iterator <it>." << std::endl;
#endif // DEBUG
									it = word_count.rbegin();
									rcnt = 1;
								}
								word_count.erase(tmpit++);
								continue;
							}
						}
						++tmpit;
					}
				}
				//只有当字数多余两个字时并且当前词的频率不小于子集的频率时才将其子集删除掉
				if (curr_wsize >= DWORD_SIZE)
					if (word_count[curr_words] >= word_count[subwords] && sub_wsize != curr_wsize) {
						word_count.erase(subwords);
#ifdef DEBUG
						std::cout << "delete subwords:" << subwords.c_str() << std::endl;
#endif // DEBUG	
					}
				if (erase_flag == 1) {
					word_count.erase(curr_words);
#ifdef DEBUG
					std::cout << "delete current_words:" << curr_words.c_str() << std::endl;
#endif // DEBUG
					
				}
			}
		}
	}
}

void CcParticiple::CcP_source_print()
{
	std::cout << "Origin essay:" << std::endl;
	std::cout << source_string.c_str() << std::endl;
}

void CcParticiple::CcP_word_print()
{
	int c = 0;
	std::cout << "words type:" << word_count.size();
	std::cout << "\tthreshold:" << threshold << std::endl;
	for (map_it it = word_count.begin(); it != word_count.end(); ++it) {
		std::cout << c++ << ":" << it->first.c_str();
		std::cout << "--" << it->second << "\t";
		if (c % 4 == 0)
			std::cout << std::endl;
	}
	std::cout << std::endl;
}

void CcParticiple::CcP_word_wback()
{
	/*1.对于查找，如果我们规定要对 M 个字组成的词进行分词，则我们首先一次读 M 个字默认它为一个词。
	  这时我们在word_count中查找该词，如果有，我们直接写入到输出文件并使用一个空格来分隔其他的词；
	  如果当前词不在word_count中，此时应该查找 M个 字的子集，看谁是一个词并按照上述规定写入到输出文件。
	例子：  
	  现在要查ABCD，如果word_count中有该词，则将它直接写入到输出文件中并将source_string的下标移到D的后面；
	  如果没有找到 ABCD，此时前后顺序组合他的所有子集即 AB，BC，CD，ABC，BCD。
	  这个时候，如果 AB 没有找到，则在 A 字的后面空格分开，如果 BC 没有找到，则继续在 B 的后面加上空格分隔。
	  以此类推，将ABCD的所有子集查找，在这个过程中也必须时刻的移动source_string的下标（指针）。
	  
	  2.由于map是按照字典树的数据结构来存储数据的，那么我们可以根据此特点来快速的查找。
	  将方法一逆向方式查找就是另一种方法，即先查找子集，如果子集存在，我们探索多一个的子集，一直到我们要
	  的 M 个字的词，此时按照贪心算法，该词是一个合适的词。如果子集不存在，则表明他们是单个字，而不是一个
	  合适的词。
	*/
	size_t cn = 0;
	std::string outstring = "";
	size_t length = source_string.size();

	for (size_t ii = 0; ii < length - 1;) {
		int punct_flag = 0;
		for (cn = words_length; cn >= DWORD_SIZE; cn -= WORD_SIZE) {
			std::string punct = "";
			std::string tmpstr = "";
			for (size_t index = ii; index < length - 1 && index - ii < cn; index += WORD_SIZE) {
				std::string word(source_string, index, WORD_SIZE);
				if (find(punctuation.begin(), punctuation.end(), word) == punctuation.end()) {
					tmpstr += word;
				}
				else {
					if (index == ii) {			//第一个字就是符号
						outstring += word;
						punct_flag = 1;
						ii += WORD_SIZE;
					}
					else
						punct = word;
					break;
				}
			}
			if (punct_flag == 1) break;
			if ((tmpstr.size() == cn) && (word_count.find(tmpstr) != word_count.end())) {
				if (punct.size() != 0) {
					outstring = outstring + tmpstr + punct;
					ii = ii + tmpstr.size() + WORD_SIZE;
				}
				else {
					outstring = outstring + tmpstr + " ";
					ii = ii + tmpstr.size();
				}
				break;
			}
		}

		if (cn == WORD_SIZE) {
			std::string word(source_string, ii, WORD_SIZE);
			outstring = outstring + word + " ";
			ii += WORD_SIZE;
		}
	}
	out << outstring.c_str();
}