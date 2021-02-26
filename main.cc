#include <iostream> 
#include <sys/types.h>	
#include <unistd.h>
#include <cstdlib>	
#include <wait.h>	
#include <vector>	
#include <string>	
#include <fstream>
using namespace std; 


//given class that forks program and executes text as bash file
bool execute(vector<string> command, string arg){

    // Convert vector of C++ strings to an array of C strings for exectp():
    vector<char *> argv;		// vector of C string pointers
    for (auto &v : command)		// for each C++ string:
	    argv.push_back(v.data());	//    add address of C-string inside it
    argv.push_back(nullptr);		// argv ends with a null pointer

    // Duplicate ourself: -1:error, 0:child, >0:parent
    const auto pid = fork();		// step #1: copy ourself
    if (pid < 0)
	return false;			// fork failure?
    if (pid == 0) {			// child?
	    execvp(argv[0], argv.data());	// step #2: replace child with new prog
	    exit(0xff);			// only got here if execvp() failed.
    }
    int status;
    wait(&status);			// step #3: wait for child to finish
    if(status == 65280){
        cerr << arg << " can’t run: ";
        for (auto x : command) {
            cerr << x << " "; 
        }
        cerr << endl;
        exit(1);
    }
    return status != 0xff00;
}

// loop through vector and strip out all empty elements
void stripElements(vector<string> &v) {
    for (uint i = 0; i < v.size(); i++) {
        if (v[i].size() == 0) {
            v.erase(v.begin() + i); 
        }
    }
}

// loop though vector and strip out one instance of each set of escape characters
void stripEscapes(vector<string> &v) {
    for (uint j = 0; j < v.size(); j++) {
        string x = v[j];
        for (uint i = 1; i < x.size()-1; i++) {
            if (x[i] == '\\') {
                v[j] = x.erase(i, 1);
                i++;
            }
        }
    }
}

//store first word and each word by spaces
vector<string> storeEach(string str, string arg){
    char previousChar = ' ';
    string word;
    // Remove trailing spaces
    while(str[str.length() -1] == ' '){
        str.erase(str.length() -1, 1);
    }
    vector<string> ret;
    bool isEscaped = false;
    int count = 0;
    for(uint i = 0; i < str.size(); i++){
        char x = str[i];
        if(x == ')' || x == '('){
            count++;
        }
        if (isEscaped) {
            isEscaped = false;
            word.push_back(x);
            continue;
        }
        if(x == '\\'){
            word.push_back(x);
            isEscaped = true; 
            continue;
        }
        if(isspace(x) || x == '(' || x == ')'){
            if(word.length() > 0){
                ret.push_back(word);
                word = "";
                
            }
        }
        else{
            word.push_back(x);
        }


        //Error handling
        if(x == ')' && previousChar != '\\' && count > 1 && i != str.length()-1){
            cerr << arg << " ERROR: paren in line without escape character \n";
            cout << "Problem line: " << str << '\n';
            exit(1);
        }
        if(x == '(' && previousChar != '\\' && count > 1){
            cerr << arg << " ERROR: paren in line without escape character \n";
            cout << "Problem line: " << str << '\n';
            exit(1);
        }
        if(str[str.length() -1] != ')'){
            cerr << arg << " ERROR: last word is not a closed paren \n";
            cout << "Problem line: " << str << '\n';
            exit(1);
        }
        if(x == '(' && ret.size() != 1){
            cerr << arg << " ERROR: second word is not an open paren \n";
            cout << "Problem line: " << str << '\n';
            exit(1);
        } 


        previousChar = x; 
    }

    if(word.length() > 0 && word != "\n"){
                ret.push_back(word);
            }

    
    stripElements(ret);
    stripEscapes(ret); 
    return ret;
}

//reads file arguments into program
vector<string> readFileToVector(const string& filename){
    ifstream source;
    source.open(filename);
    vector<string> lines;
    string line;
    while (getline(source, line)){
        lines.push_back(line);
    }
    return lines;
    }

int main(int argc, char **argv){
    string str;
    //single txt doc/argument
    if(argc == 1){
        vector<vector<string>> v;
        while(getline(cin, str)){
            v.push_back(storeEach(str, argv[0])); 
        }
        for(uint i = 0; i < v.size(); i++){
            execute(v[i], argv[0]);
        }
    }

    //multiple text docs/arguments
    int size = argc;

    if(argc > 1){
        for(int i = 1; i < size; i++){

            if(access(argv[i], F_OK) != 0){
                cerr << argv[0] << " a filename argument can’t be opened for reading" << '\n';
                exit(1);
            }

            string argument(argv[i]);
            vector<string> vec = readFileToVector(argument);
            for(uint j = 0; j < vec.size(); j++){    
                auto retV = storeEach(vec[j], argv[0]); 
                execute(retV, argv[0]);
            }
        }
    }
    return 0;
} 

