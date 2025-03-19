#pragma once

#include <stdio.h>
#include <jni.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <dlfcn.h>
#include <dirent.h>
#include <stdlib.h>
#include <string>
#include <mutex>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

namespace Utils {
	inline void Getfilepath(const char* path, const char* filename, char* filepath)
	{
		strcpy(filepath, path);
		if (filepath[strlen(path) - 1] != '/')
			strcat(filepath, "/");
		strcat(filepath, filename);
	}
	inline std::vector<std::string> listDir(const char* path)
	{
		std::vector<std::string> result;

		DIR* dir;
		struct dirent* dirinfo;
		struct stat statbuf;
		char filepath[256] = { 0 };
		lstat(path, &statbuf);
		
		if (S_ISREG(statbuf.st_mode))//判断是否是常规文件
		{
			result.push_back(path);
		}
		else if (S_ISDIR(statbuf.st_mode))//判断是否是目录
		{
			if ((dir = opendir(path)) == NULL)
				return result;
			while ((dirinfo = readdir(dir)) != NULL)
			{
				Getfilepath(path, dirinfo->d_name, filepath);
				if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0)//判断是否是特殊目录
					continue;
				auto list = listDir(filepath);
				result.insert(result.end(), list.begin(), list.end());
			}
			closedir(dir);
		}
		return result;
	}

	template <typename TRet>
	inline std::string asString(TRet v) {
		std::stringstream os;
		os << v;
		return os.str();
	}



	inline string& replace_all(string& str, const   string& old_value, const   string& new_value)
	{
		while (true) {
			string::size_type   pos(0);
			if ((pos = str.find(old_value)) != string::npos)
				str.replace(pos, old_value.length(), new_value);
			else   break;
		}
		return   str;
	}

	inline void copyFile(const char* from, const char* to) {

		char buff[1024];
		int len;

		FILE* in, * out;

		in = fopen(from, "r");
		out = fopen(to, "w+");

		while ((len = fread(buff, 1, sizeof(buff), in)) != 0)
		{
			fwrite(buff, 1, len, out);
		}

		fclose(in);
		fclose(out);
	}

	inline int folder_mkdirs(const char* folder_path)
	{
		if (!access(folder_path, F_OK)) {                        /* 判断目标文件夹是否存在 */
			return 1;
		}

		char path[256];                                        /* 目标文件夹路径 */
		char* path_buf;                                        /* 目标文件夹路径指针 */
		char temp_path[256];                                   /* 存放临时文件夹路径 */
		char* temp;                                            /* 单级文件夹名称 */
		int temp_len;                                          /* 单级文件夹名称长度 */

		memset(path, 0, sizeof(path));
		memset(temp_path, 0, sizeof(temp_path));
		strcat(path, folder_path);
		path_buf = path;

		while ((temp = strsep(&path_buf, "/")) != NULL) {        /* 拆分路径 */
			temp_len = strlen(temp);
			if (0 == temp_len) {
				continue;
			}
			strcat(temp_path, "/");
			strcat(temp_path, temp);
			printf("temp_path = %s\n", temp_path);
			if (-1 == access(temp_path, F_OK)) {                 /* 不存在则创建 */
				if (-1 == mkdir(temp_path, 0755)) {
					return 2;
				}
			}
		}
		return 1;
	}

	inline int ReadHttpStatus(int sock)
	{
		char c;
		char buff[1024] = "", * ptr = buff + 1;
		int bytes_received, status;

		bytes_received = recv(sock, ptr, 1, 0);
		while (bytes_received)
		{

			if ((ptr[-1] == '\r') && (*ptr == '\n')) break;
			ptr++;
			bytes_received = recv(sock, ptr, 1, 0);
		}
		*ptr = 0;
		ptr = buff + 1;

		sscanf(ptr, "%*s %d ", &status);

		return (bytes_received > 0) ? status : 0;

	}

	//the only filed that it parsed is 'Content-Length' 
	inline int ParseHeader(int sock)
	{
		char c;
		char buff[1024] = "", * ptr = buff + 4;
		int bytes_received, status;
		bytes_received = recv(sock, ptr, 1, 0);
		while (bytes_received)
		{

			if (
				(ptr[-3] == '\r') && (ptr[-2] == '\n') &&
				(ptr[-1] == '\r') && (*ptr == '\n')
				) break;
			ptr++;
			bytes_received = recv(sock, ptr, 1, 0);
		}
		*ptr = 0;
		ptr = buff + 4;
		if (bytes_received)
		{
			ptr = strstr(ptr, "Content-Length:");
			if (ptr)
			{
				sscanf(ptr, "%*s %d", &bytes_received);
			}
			else
				bytes_received = -1; //unknown size


		}
		return  bytes_received;
	}

	inline int download(const char* domain,const char* path,const char * save)
	{


		int sock, bytes_received;
		char send_data[1024], recv_data[1024], * p;
		struct sockaddr_in server_addr;
		struct hostent* he;


		he = gethostbyname(domain);
		if (he == NULL)
		{
			exit(1);
		}

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			perror("Socket");
			exit(1);
		}
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(80);
		server_addr.sin_addr = *((struct in_addr*)he->h_addr);
		bzero(&(server_addr.sin_zero), 8);

		printf("Connecting ...\n");
		if (connect(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1)
		{
			perror("Connect");
			exit(1);
		}

		printf("Sending data ...\n");

		snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, domain);

		if (send(sock, send_data, strlen(send_data), 0) == -1)
		{

		}


		int contentlengh;

		if (ReadHttpStatus(sock) && (contentlengh = ParseHeader(sock)))
		{
			int bytes = 0;
			FILE* fd = fopen(save, "wb");


			bytes_received = recv(sock, recv_data, 1024, 0);
			while (bytes_received)
			{
				if (bytes_received == -1)
				{
					perror("recieve");
					exit(3);
				}
				fwrite(recv_data, 1, bytes_received, fd);
				bytes += bytes_received;
				printf("Bytes recieved: %d from %d\n", bytes, contentlengh);
				if (bytes == contentlengh)
					break;
				bytes_received = recv(sock, recv_data, 1024, 0);
			}
			fclose(fd);
		}



		close(sock);
		printf("\n\nDone.\n\n");
		return 0;
	}

	inline int match(const char* main, const char* sub)
	{
		int count = 0;
		for (int i = 0; main[i] != '\0'; i++)
		{
			for (int j = 0; sub[j] != '\0'; j++)
			{
				if (main[i + j] != sub[j]) { break; }
				if (sub[j + 1] == '\0') { count++; }  //core
			}
		}
		return count;
	}

	

	inline bool DeleteFile(const char* path)
	{
		DIR* dir;
		struct dirent* dirinfo;
		struct stat statbuf;
		char filepath[256] = { 0 };
		lstat(path, &statbuf);

		if (S_ISREG(statbuf.st_mode))//判断是否是常规文件
		{
			remove(path);
		}
		else if (S_ISDIR(statbuf.st_mode))//判断是否是目录
		{
			if ((dir = opendir(path)) == NULL)
				return 1;
			while ((dirinfo = readdir(dir)) != NULL)
			{
				Getfilepath(path, dirinfo->d_name, filepath);
				if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0)//判断是否是特殊目录
					continue;
				DeleteFile(filepath);
				rmdir(filepath);
			}
			closedir(dir);
		}
		return 0;
	}



	static bool WriteStringToFile(const char* Path, std::string const& str)  __attribute__((always_inline));

	static bool WriteStringToFile(const char* Path, std::string const& str) {
		if (access(Path, F_OK) != 0) {
			int fd = open(Path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH);
			close(fd);
		}
		if (access(Path, F_OK | R_OK | W_OK) != 0) {
			return false;
		}
		std::ofstream OsWrite(Path, std::ofstream::app);
		OsWrite << str << std::endl;
		OsWrite.close();
		return true;
	}

	static std::vector<std::string> split(std::string str, std::string pattern)  __attribute__((always_inline));

	static std::vector<std::string> split(std::string str, std::string pattern)
	{
		std::string::size_type pos;
		std::vector<std::string> result;
		str += pattern;
		int size = str.size();

		for (int i = 0; i < size; i++)
		{
			pos = str.find(pattern, i);
			if (pos < size)
			{
				std::string s = str.substr(i, pos - i);
				result.push_back(s);
				i = pos + pattern.size() - 1;
			}
		}
		return result;
	}

	static std::string readFileIntoString(const char* Path)  __attribute__((always_inline));

	static std::string readFileIntoString(const char* Path)
	{
		std::ifstream ifile(Path,std::ios::binary);
		std::ostringstream buf;
		char ch;
		while (buf && ifile.get(ch))
			buf.put(ch);
		return buf.str();
	}
	static char* itoa(int num, char* str, int radix)   __attribute__((always_inline));

	static char* itoa(int num, char* str, int radix)
	{
		/* 索引表 */
		char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		unsigned unum; /* 中间变量 */
		int i = 0, j, k;
		/* 确定unum的值 */
		if (radix == 10 && num < 0) /* 十进制负数 */
		{
			unum = (unsigned)-num;
			str[i++] = '-';
		}
		else unum = (unsigned)num; /* 其它情况 */
		/* 逆序 */
		do
		{
			str[i++] = index[unum % (unsigned)radix];
			unum /= radix;
		} while (unum);
		str[i] = '\0';
		/* 转换 */
		if (str[0] == '-') k = 1; /* 十进制负数 */
		else k = 0;
		/* 将原来的“/2”改为“/2.0”，保证当num在16~255之间，radix等于16时，也能得到正确结果 */
		char temp;
		for (j = k; j <= (i - k - 1) / 2.0; j++)
		{
			temp = str[j];
			str[j] = str[i - j - 1];
			str[i - j - 1] = temp;
		}
		return str;
	}

	static	string  int64_to_string(uint64_t _num) __attribute__((always_inline));

	static	string  int64_to_string(uint64_t _num)
	{

		const unsigned long long  _div = 10000000;

		unsigned long long    upper_part = _num / _div;
		unsigned long long  lower_part = _num - upper_part * _div;

		char buf[100];

		memset(buf, 0, sizeof(char) * 100);

		itoa((int)upper_part, buf, 10);
		string ret = buf;

		memset(buf, 0, sizeof(char) * 100);
		itoa((int)lower_part, buf, 10);
		ret.append(buf);


		while (ret[0] == '0')
		{
			ret = ret.substr(1, -1);

		}

		return ret;

	}

	inline char ahextoi(const char* hex_str)
	{
		int r = 0;
		if (hex_str)
			sscanf(hex_str, "%x", &r);
		return (char)r;
	}

	inline std::string hex2string(std::string const& hex){
		auto sv = Utils::split(hex, " ");
		std::string res;
		for (auto i : sv) {
			char t = ahextoi(i.c_str());
			res.push_back(t);
		}
		return res;
	}

	inline unsigned int pos_of_char(const unsigned char chr) {
		//
		// Return the position of chr within base64_encode()
		//

		if (chr >= 'A' && chr <= 'Z') return chr - 'A';
		else if (chr >= 'a' && chr <= 'z') return chr - 'a' + ('Z' - 'A') + 1;
		else if (chr >= '0' && chr <= '9') return chr - '0' + ('Z' - 'A') + ('z' - 'a') + 2;
		else if (chr == '+' || chr == '-') return 62; // Be liberal with input and accept both url ('-') and non-url ('+') base 64 characters (
		else if (chr == '/' || chr == '_') return 63; // Ditto for '/' and '_'
		else return 0;
	}

	inline std::string base64_decode(std::string encoded_string, bool remove_linebreaks) {
		//
		// decode(…) is templated so that it can be used with String = const std::string&
		// or std::string_view (requires at least C++17)
		//

		if (encoded_string.empty()) return std::string();

		if (remove_linebreaks) {

			std::string copy(encoded_string);

			copy.erase(std::remove(copy.begin(), copy.end(), '\n'), copy.end());

			return base64_decode(copy, false);
		}

		size_t length_of_string = encoded_string.length();
		size_t pos = 0;

		//
		// The approximate length (bytes) of the decoded string might be one or
		// two bytes smaller, depending on the amount of trailing equal signs
		// in the encoded string. This approximation is needed to reserve
		// enough space in the string to be returned.
		//
		size_t approx_length_of_decoded_string = length_of_string / 4 * 3;
		std::string ret;
		ret.reserve(approx_length_of_decoded_string);

		while (pos < length_of_string) {
			
			size_t pos_of_char_1 = pos_of_char(encoded_string[pos + 1]);

			//
			// Emit the first output byte that is produced in each chunk:
			//
			ret.push_back(static_cast<std::string::value_type>(((pos_of_char(encoded_string[pos + 0])) << 2) + ((pos_of_char_1 & 0x30) >> 4)));

			if ((pos + 2 < length_of_string) &&  // Check for data that is not padded with equal signs (which is allowed by RFC 2045)
				encoded_string[pos + 2] != '=' &&
				encoded_string[pos + 2] != '.'            // accept URL-safe base 64 strings, too, so check for '.' also.
				)
			{
				//
				// Emit a chunk's second byte (which might not be produced in the last chunk).
				//
				unsigned int pos_of_char_2 = pos_of_char(encoded_string[pos + 2]);
				ret.push_back(static_cast<std::string::value_type>(((pos_of_char_1 & 0x0f) << 4) + ((pos_of_char_2 & 0x3c) >> 2)));

				if ((pos + 3 < length_of_string) &&
					encoded_string[pos + 3] != '=' &&
					encoded_string[pos + 3] != '.'
					)
				{
					//
					// Emit a chunk's third byte (which might not be produced in the last chunk).
					//
					ret.push_back(static_cast<std::string::value_type>(((pos_of_char_2 & 0x03) << 6) + pos_of_char(encoded_string[pos + 3])));
				}
			}

			pos += 4;
		}

		return ret;
	}

	static const char* base64_chars[2] = {
			 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			 "abcdefghijklmnopqrstuvwxyz"
			 "0123456789"
			 "+/",

			 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			 "abcdefghijklmnopqrstuvwxyz"
			 "0123456789"
			 "-_" };
	inline std::string base64_encode(unsigned char const* bytes_to_encode, size_t in_len, bool url) {

		size_t len_encoded = (in_len + 2) / 3 * 4;

		unsigned char trailing_char = url ? '.' : '=';

		//
		// Choose set of base64 characters. They differ
		// for the last two positions, depending on the url
		// parameter.
		// A bool (as is the parameter url) is guaranteed
		// to evaluate to either 0 or 1 in C++ therefore,
		// the correct character set is chosen by subscripting
		// base64_chars with url.
		//
		const char* base64_chars_ = base64_chars[url];

		std::string ret;
		ret.reserve(len_encoded);

		unsigned int pos = 0;

		while (pos < in_len) {
			ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0xfc) >> 2]);

			if (pos + 1 < in_len) {
				ret.push_back(base64_chars_[((bytes_to_encode[pos + 0] & 0x03) << 4) + ((bytes_to_encode[pos + 1] & 0xf0) >> 4)]);

				if (pos + 2 < in_len) {
					ret.push_back(base64_chars_[((bytes_to_encode[pos + 1] & 0x0f) << 2) + ((bytes_to_encode[pos + 2] & 0xc0) >> 6)]);
					ret.push_back(base64_chars_[bytes_to_encode[pos + 2] & 0x3f]);
				}
				else {
					ret.push_back(base64_chars_[(bytes_to_encode[pos + 1] & 0x0f) << 2]);
					ret.push_back(trailing_char);
				}
			}
			else {

				ret.push_back(base64_chars_[(bytes_to_encode[pos + 0] & 0x03) << 4]);
				ret.push_back(trailing_char);
				ret.push_back(trailing_char);
			}

			pos += 3;
		}


		return ret;
	}

	inline std::string encode(std::string const& s, bool url) {
		return base64_encode(reinterpret_cast<const unsigned char*>(s.data()), s.length(), url);
	}
	inline std::string base64_encode(std::string const& s, bool url) {
		return encode(s, url);
	}
}
