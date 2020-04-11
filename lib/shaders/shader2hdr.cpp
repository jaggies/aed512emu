#include <iostream>
#include <fstream>
#include <algorithm>
#include <libgen.h>

using namespace std;

int main(int argc, char** argv)
{
	if (argc < 3) {
		cerr << "Usage: " << argv[0] << " <basefilename> <headername>" << endl;
		return 0;
	}
	const string baseFileName = basename(argv[1]);
	const string headerName = argv[2];
	const string fragmentName = baseFileName + ".frag";
	const string vertexName = baseFileName + ".vert";

  	ifstream fragment(fragmentName);
  	if (!fragment) {
		cerr << "Can't open fragment shader '" << fragmentName << "'" << endl;
		return 0;
	}

  	ifstream vertex(vertexName);
	if (!vertex) {
		cerr << "Can't open vertex shader '" << vertexName << "'" << endl;
		return 0;
	}

	ofstream header(headerName);
	if (!header) {
		cerr << "Can't write header file '" << headerName << "'" << endl;
		return 0;
	}

	// Create header
	string def = headerName;
	transform(def.begin(), def.end(), def.begin(), [](uint8_t c) { return c == '.' ? '_' : toupper(c); });
	header << "/* Auto-generated header file '" << headerName << "' */" << endl;
	header << "#ifndef " << def << endl;
	header << "#define " << def << endl;
	header << endl;

	// Import vertex shader
	string line;
	header << baseFileName << "VertexShader = \\" << endl;
    while (getline(vertex, line)) {
		header << "\"" << line << "\\n\"" << endl;
    }
	header << ";" << std::endl;
	fragment.close();
	header << "#endif // " << def << endl;
}
