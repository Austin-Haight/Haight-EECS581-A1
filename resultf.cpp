/*prompt: write a function that, given a string, 
can detect if an IPV4 address is in that string, including port, 
and returns the address and port, 
do not use regex or address parsing libraries

model: GPT-6 Luna
date: 9/25/2026

all comments were added by me to demonstrate understanding, no code is modified from the LLM output
*/

#include <cctype>
#include <iostream>
#include <string>

struct IPv4Endpoint {
	std::string address;
	unsigned int port;
};

namespace {

//attempts to parse a decimal number of a maximum size and digit length from a string
bool parseDecimal(
        const std::string& text, //string to parse
        std::size_t& position, //position to start parsing from
		std::size_t maxDigits, //the maximum number of consecutive digits to parse before failing
        unsigned int maxValue, //the maximum number to return before failing
		unsigned int& value //location to store the parsed number in
        ) {
	const std::size_t start = position;
	value = 0;

	while (position < text.size() &&
		   std::isdigit(static_cast<unsigned char>(text[position]))) {
		if (position - start == maxDigits) { //fail if we hit max digits
			return false;
		}
		value = value * 10 + static_cast<unsigned int>(text[position] - '0');//actual parsing step
		if (value > maxValue) { //fail if we hit the maximum size
			return false;
		}
		++position;
	}

	return position != start; //if the first character fails, the function fails
}

//returns true if the character is alphanumeric, false otherwise
bool isTokenCharacter(char character) {
	return std::isalnum(static_cast<unsigned char>(character)) ||
		   character == '_';
}

}
//attempts to find an ipv4 address with a port in the input string, notably fails if a valid address has no port
bool findIPv4WithPort(const std::string& text, IPv4Endpoint& endpoint) {
	for (std::size_t start = 0; start < text.size(); ++start) {
        //skips any obvious non-addresses and malformed addresses, notably will reject any addresses that are directly preceeded by a letter
		if (!std::isdigit(static_cast<unsigned char>(text[start])) ||
			(start > 0 && (isTokenCharacter(text[start - 1]) ||
						   text[start - 1] == '.'))) {
			continue;
		}

        //begins an attempt to parse an address at the current location
		std::size_t position = start;
		unsigned int octet = 0;
		bool valid = true;

        //attempts to match the non-port part of the address
		for (int part = 0; part < 4; ++part) {
			if (!parseDecimal(text, position, 3, 255, octet)) {//individual octet check
				valid = false;
				break;
			}
			if (part < 3) {//check to make sure 4 octets were found
				if (position >= text.size() || text[position] != '.') {
					valid = false;
					break;
				}
				++position;
			}
		}
		if (!valid || position >= text.size() || text[position] != ':') {//this check is where addresses without ports are discarded
			continue;
		}

		const std::size_t addressEnd = position;
		++position;

        //begins an attempt to match the port at the end of an address
		unsigned int port = 0;
		if (!parseDecimal(text, position, 5, 65535, port)) {
			continue;
		}
		if (position < text.size() &&
			(isTokenCharacter(text[position]) || text[position] == '.' ||
			 text[position] == ':')) {
			continue;
		}

        //if both matches succeeded, store the address and port, then return
		endpoint.address = text.substr(start, addressEnd - start);
		endpoint.port = port;
		return true;
	}

	return false;
}

int main() {
	std::string text;
	std::getline(std::cin, text);

	IPv4Endpoint endpoint;
	if (!findIPv4WithPort(text, endpoint)) {
		return 1;
	}

	std::cout << endpoint.address << ':' << endpoint.port << '\n';
	return 0;
}

