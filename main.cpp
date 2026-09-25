/*
primary changes:
-accept addresses that do not attempt to provide a port
-ignore letters directly preceeding and following address
-more interaction and descriptive outputs in main
-port output should be signed to allow for -1 null port
-reject numbers with leading zeros
-isTokenCharacter is redundant and overrejects, replace with std::isdigit
-output address as a decimal as well to avoid redundant parsing
*/
#include <cctype>
#include <iostream>
#include <string>

struct IPv4Endpoint {
	std::string address;
    unsigned int numeric;
	int port;
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

        //invalidate leading zeros
        if(position!=start && value==0){
            return false;
        }
        
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

}
//attempts to find an ipv4 address with a port in the input string, notably fails if a valid address has no port
bool findIPv4WithPort(const std::string& text, IPv4Endpoint& endpoint) {
	for (std::size_t start = 0; start < text.size(); ++start) {
        //skips any obvious non-addresses and malformed addresses
		if (!std::isdigit(static_cast<unsigned char>(text[start])) ||
			(start > 0 && (isdigit(static_cast<unsigned char>(text[start - 1])) ||
						   text[start - 1] == '.'))) {
			continue;
		}

        //begins an attempt to parse an address at the current location
		std::size_t position = start;
		unsigned int octet = 0;
        unsigned int addrNumeric =0;
		bool valid = true;

        //attempts to match the non-port part of the address
		for (int part = 0; part < 4; ++part) {
			if (!parseDecimal(text, position, 3, 255, octet)) {//individual octet check
				valid = false;
				break;
			}else{
                addrNumeric=addrNumeric<<8;
                addrNumeric+=octet;
            }
			if (part < 3) {//check to make sure 4 octets were found
				if (position >= text.size() || text[position] != '.') {
					valid = false;
					break;
				}
				++position;
			}
		}
		if (!valid || text[position] == '.' || std::isdigit(static_cast<unsigned char>(text[position]))) {
			continue;
		}

		const std::size_t addressEnd = position;

        //accept portless addresses
        if(position >= text.size() || text[position] != ':'){
            endpoint.address = text.substr(start, addressEnd - start);
            endpoint.numeric = addrNumeric;
		    endpoint.port = -1;
		    return true;
        }
		++position;

        //begins an attempt to match the port at the end of an address
		unsigned int port = 0;
		if (!parseDecimal(text, position, 5, 65535, port)) {
			continue;
		}
		if (position < text.size() &&
			(std::isdigit(static_cast<unsigned char>(text[position])) || text[position] == '.' ||
			 text[position] == ':')) {
			continue;
		}

        //if both matches succeeded, store the address and port, then return
		endpoint.address = text.substr(start, addressEnd - start);
        endpoint.numeric = addrNumeric;
		endpoint.port = port;
		return true;
	}

	return false;
}

int main() {
	std::string text;

    while(1){
        std::cout<<"Enter a string (or 'END' to quit): ";
	    std::getline(std::cin, text);

        if(text=="END"){
            break;
        }

	    IPv4Endpoint endpoint;
	    if (!findIPv4WithPort(text, endpoint)) {
		    continue;
	    }


	    std::cout << "Extracted IPv4 address: " << endpoint.address << " (decimal value: " << endpoint.numeric << ", port: ";
        if(endpoint.port<0){
            std::cout<<"none)\n";
        }else{
            std::cout<<endpoint.port<<")\n";
        }

    }
	return 0;
}

