#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace std;
/*
ID: 20190554
Name: Mennatullah Rashed Ahmed

ID: 20190180
Name: Hasnaa Mounir saaed

ID: 20190353
Name: Omar Khaled Mohy
*/


/*
{ Sample program
  in TINY language
  compute factorial
}

read x; {input an integer}
if 0<x then {compute only if x>=1}
  fact:=1;
  repeat
    fact := fact * x;
    x:=x-1
  until x=0;
  write fact {output factorial}
end
*/

// sequence of statements separated by ;
// no procedures - no declarations
// all variables are integers
// variables are declared simply by assigning values to them :=
// if-statement: if (boolean) then [else] end
// repeat-statement: repeat until (boolean)
// boolean only in if and repeat conditions < = and two mathematical expressions
// math expressions integers only, + - * / ^
// I/O read write
// Comments {}

////////////////////////////////////////////////////////////////////////////////////
// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

bool StartsWith(const char *a, const char *b) {
    int nb = strlen(b);
    return strncmp(a, b, nb) == 0;
}

void Copy(char *a, const char *b, int n = 0) {
    if (n > 0) {
        strncpy(a, b, n);
        a[n] = 0;
    } else strcpy(a, b);
}

void AllocateAndCopy(char **a, const char *b) {
    if (b == 0) {
        *a = 0;
        return;
    }
    int n = strlen(b);
    *a = new char[n + 1];
    strcpy(*a, b);
}

////////////////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////////////////
bool ENDFILE_FLAGGED = false;
#define MAX_LINE_LENGTH 10000

struct InFile {
    FILE *file;
    int cur_line_num;

    char line_buf[MAX_LINE_LENGTH];
    int cur_ind, cur_line_size;

    InFile(const char *str) {
        file = 0;
        if (str) file = fopen(str, "r");
        cur_line_size = 0;
        cur_ind = 0;
        cur_line_num = 0;
    }

    ~InFile() { if (file) fclose(file); }

    void SkipSpaces() {
        while (cur_ind < cur_line_size) {
            char ch = line_buf[cur_ind];
            if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') break;
            cur_ind++;
        }
    }

    bool SkipUpto(const char *str) {
        while (true) {
            SkipSpaces();
            while (cur_ind >= cur_line_size) {
                if (!GetNewLine()) return false;
                SkipSpaces();
            }

            if (StartsWith(&line_buf[cur_ind], str)) {
                cur_ind += strlen(str);
                return true;
            }
            cur_ind++;
        }
        return false;
    }

    bool GetNewLine() {
        cur_ind = 0;
        line_buf[0] = 0;
        if (!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size = strlen(line_buf);
        if (cur_line_size == 0) return false; // End of file
        cur_line_num++;
        return true;
    }

    char *GetNextTokenStr() {
        if (ENDFILE_FLAGGED) {
            return 0;
        }
        SkipSpaces();
        while (cur_ind >= cur_line_size) {
            if (!GetNewLine()) {
                ENDFILE_FLAGGED = true;
                return 0;
            }
            SkipSpaces();
        }
        return &line_buf[cur_ind];
    }

    void Advance(int num) {
        cur_ind += num;
    }
};

struct OutFile {
    FILE *file;

    OutFile(const char *str) {
        file = 0;
        if (str) file = fopen(str, "w");
    }

    ~OutFile() { if (file) fclose(file); }

    void Out(const char *s) {
        fprintf(file, "%s\n", s);
        fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo {
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char *in_str, const char *out_str, const char *debug_str)
            : in_file(in_str), out_file(out_str), debug_file(debug_str) {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType {
    IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
    ASSIGN, EQUAL, LESS_THAN,
    PLUS, MINUS, TIMES, DIVIDE, POWER,
    SEMI_COLON,
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    ID, NUM,
    ENDFILE, ERROR
};

// Used for debugging only /////////////////////////////////////////////////////////
const char *TokenTypeStr[] =
        {
                "If", "Then", "Else", "End", "Repeat", "Until", "Read", "Write",
                "Assign", "Equal", "LessThan",
                "Plus", "Minus", "Times", "Divide", "Power",
                "SemiColon",
                "LeftParen", "RightParen",
                "LeftBrace", "RightBrace",
                "ID", "Num",
                "EndFile", "Error"
        };

struct Token {
    TokenType type;
    char str[MAX_TOKEN_LEN + 1];

    Token() {
        str[0] = 0;
        type = ERROR;
    }

    Token(TokenType _type, const char *_str) {
        type = _type;
        Copy(str, _str);
    }
};

const Token reserved_words[] =
        {
                Token(IF, "if"),
                Token(THEN, "then"),
                Token(ELSE, "else"),
                Token(END, "end"),
                Token(REPEAT, "repeat"),
                Token(UNTIL, "until"),
                Token(READ, "read"),
                Token(WRITE, "write")
        };
const int num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment
const Token symbolic_tokens[] =
        {
                Token(ASSIGN, ":="),
                Token(EQUAL, "="),
                Token(LESS_THAN, "<"),
                Token(PLUS, "+"),
                Token(MINUS, "-"),
                Token(TIMES, "*"),
                Token(DIVIDE, "/"),
                Token(POWER, "^"),
                Token(SEMI_COLON, ";"),
                Token(LEFT_PAREN, "("),
                Token(RIGHT_PAREN, ")"),
                Token(LEFT_BRACE, "{"),
                Token(RIGHT_BRACE, "}")
        };
const int num_symbolic_tokens = sizeof(symbolic_tokens) / sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch) { return (ch >= '0' && ch <= '9'); }

inline bool IsLetter(char ch) { return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')); }

inline bool IsLetterOrUnderscore(char ch) { return (IsLetter(ch) || ch == '_'); }


void GetNextToken(CompilerInfo *pci, Token *ptoken) {
    ptoken->type = ERROR;
    ptoken->str[0] = 0;

    int i;
    char *s = pci->in_file.GetNextTokenStr();
    if (!s) {
        ptoken->type = ENDFILE;
        ptoken->str[0] = 0;
        return;
    }

    for (i = 0; i < num_symbolic_tokens; i++) {
        if (StartsWith(s, symbolic_tokens[i].str))
            break;
    }

    if (i < num_symbolic_tokens) {
        if (symbolic_tokens[i].type == LEFT_BRACE) {
            pci->in_file.Advance(strlen(symbolic_tokens[i].str));
            if (!pci->in_file.SkipUpto(symbolic_tokens[i + 1].str)) return;
            return GetNextToken(pci, ptoken);
        }
        ptoken->type = symbolic_tokens[i].type;
        Copy(ptoken->str, symbolic_tokens[i].str);
    } else if (IsDigit(s[0])) {
        int j = 1;
        while (IsDigit(s[j])) j++;

        ptoken->type = NUM;
        Copy(ptoken->str, s, j);
    } else if (IsLetterOrUnderscore(s[0])) {
        int j = 1;
        while (IsLetterOrUnderscore(s[j])) j++;

        ptoken->type = ID;
        Copy(ptoken->str, s, j);

        for (i = 0; i < num_reserved_words; i++) {
            if (Equals(ptoken->str, reserved_words[i].str)) {
                ptoken->type = reserved_words[i].type;
                break;
            }
        }
    }

    int len = strlen(ptoken->str);
    if (len > 0) pci->in_file.Advance(len);
}

vector<Token> tokens;

void Scanner(CompilerInfo *compilerInfo) {

    Token ptoken;
    while (true) {
        GetNextToken(compilerInfo, &ptoken);
        tokens.push_back(ptoken);
        if (ptoken.type == ENDFILE || ptoken.type == ERROR) {
            break;
        }
    }
    for (Token x: tokens) {
        cout << TokenTypeStr[x.type] << " " << x.str << endl;
    }

}

////////////////////////////////////////////////////////////////////////////////////
// Parser //////////////////////////////////////////////////////////////////////////

// program -> stmtseq
// stmtseq -> stmt { ; stmt }
// stmt -> ifstmt | repeatstmt | assignstmt | readstmt | writestmt
// ifstmt -> if exp then stmtseq [ else stmtseq ] end
// repeatstmt -> repeat stmtseq until expr
// assignstmt -> identifier := expr
// readstmt -> read identifier
// writestmt -> write expr
// expr -> mathexpr [ (<|=) mathexpr ]
// mathexpr -> term { (+|-) term }    left associative
// term -> factor { (*|/) factor }    left associative
// factor -> newexpr { ^ newexpr }    right associative
// newexpr -> ( mathexpr ) | number | identifier

enum NodeKind {
    IF_NODE, REPEAT_NODE, ASSIGN_NODE, READ_NODE, WRITE_NODE,
    OPER_NODE, NUM_NODE, ID_NODE
};

// Used for debugging only /////////////////////////////////////////////////////////
const char *NodeKindStr[] =
        {
                "If", "Repeat", "Assign", "Read", "Write",
                "Oper", "Num", "ID"
        };

enum ExprDataType {
    VOID, INTEGER, BOOLEAN
};

// Used for debugging only /////////////////////////////////////////////////////////
const char *ExprDataTypeStr[] =
        {
                "Void", "Integer", "Boolean"
        };

#define MAX_CHILDREN 3

struct TreeNode {
    TreeNode *child[MAX_CHILDREN];
    TreeNode *sibling; // used for sibling statements only

    NodeKind node_kind;

    union {
        TokenType oper;
        int num;
        char *id;
    }; // defined for expression/int/identifier only
    ExprDataType expr_data_type; // defined for expression/int/identifier only

    int line_num;

    TreeNode() {
        int i;
        for (i = 0; i < MAX_CHILDREN; i++) child[i] = 0;
        sibling = 0;
        expr_data_type = VOID;
    }
};

struct ParseInfo {
    Token next_token;
};

void PrintTree(TreeNode *node, int sh = 0) {
    int i, NSH = 3;
    for (i = 0; i < sh; i++) printf(" ");

    printf("[%s]", NodeKindStr[node->node_kind]);

    if (node->node_kind == OPER_NODE) printf("[%s]", TokenTypeStr[node->oper]);
    else if (node->node_kind == NUM_NODE) printf("[%d]", node->num);
    else if (node->node_kind == ID_NODE || node->node_kind == READ_NODE || node->node_kind == ASSIGN_NODE)
        printf("[%s]", node->id);

    if (node->expr_data_type != VOID) printf("[%s]", ExprDataTypeStr[node->expr_data_type]);

    printf("\n");

    for (i = 0; i < MAX_CHILDREN; i++) if (node->child[i]) PrintTree(node->child[i], sh + NSH);
    if (node->sibling) PrintTree(node->sibling, sh);
}

class Parser {
private:
    TreeNode *parseTreeRoot;

public:
    ParseInfo parseInfo;
    CompilerInfo *compilerInfo;

    Parser(CompilerInfo *compilerInfo) {
        this->compilerInfo = compilerInfo;
    };

    TreeNode *Parse() {
        //We get the first token and store in parseInfo
        GetNextToken(compilerInfo, &parseInfo.next_token);
        //Start from program non-terminal in grammer
        parseTreeRoot = _program();
        return parseTreeRoot;
    }

    TreeNode *_program() {

        //we start by going to stmtseq as its the start of program R.H.S.
        TreeNode *tree;
        try {
            tree = _stmtseq();
        } catch (...) {
            cout << "ERROR" << endl;
            return nullptr;
        }
        //After building the program by evaluating terminals and non-terminals
        //We check for EOF token
        if (parseInfo.next_token.type == ENDFILE) {
            return tree;
        } else {
            return tree; //test
            //return nullptr;
        }
    }


    TreeNode *_stmtseq() {
        //stmtseq evaluates to either stmt  or  stmt followed by ; and other stmts ( 1 or more )
        // first stmt R.H.S.
        TreeNode *subtree;
        subtree = _stmt();

        // other stmts if present
        if (parseInfo.next_token.type == SEMI_COLON) {
            TreeNode *cur = subtree;
            while (parseInfo.next_token.type != ENDFILE) {
                if (parseInfo.next_token.type == SEMI_COLON) {
                    GetNextToken(compilerInfo, &parseInfo.next_token);
                }
                if (parseInfo.next_token.type == END || parseInfo.next_token.type == ENDFILE ||
                    parseInfo.next_token.type == UNTIL) {
                    break;
                }
                TreeNode *new_sub_tree = _stmt();
                cur->sibling = new_sub_tree;
                cur = new_sub_tree;

            }
        }
        return subtree;
    }


    TreeNode *_stmt() {
        //cout << parseInfo.next_token.str << endl;

        TreeNode *subtree = new TreeNode;
        if (parseInfo.next_token.type == IF) {
            subtree = _ifstmt();
        } else if (parseInfo.next_token.type == READ) {
            subtree = _readstmt();
        } else if (parseInfo.next_token.type == REPEAT) { // TODO
            subtree = _repeatstmt();
        } else if (parseInfo.next_token.type == ASSIGN || parseInfo.next_token.type == ID) { // TODO
            subtree = _assignstmt();
        } else if (parseInfo.next_token.type == WRITE) { // TODO
            subtree = _writestmt();
        }

        return subtree;
    }

    TreeNode *_ifstmt() {
        // ifstmt -> if exp then stmtseq [ else stmtseq ] end
        TreeNode *subtree = new TreeNode;
        subtree->line_num = compilerInfo->in_file.cur_line_num;
        subtree->node_kind = IF_NODE;

        GetNextToken(compilerInfo, &parseInfo.next_token);
        subtree->child[0] = _expr();

        bool prev = false;

        if (parseInfo.next_token.type == THEN) {
            GetNextToken(compilerInfo, &parseInfo.next_token);
            subtree->child[1] = _stmtseq();
            prev = true;
        } else {
            throw 0;
        }

        if (prev) {
            if (parseInfo.next_token.type == ELSE) {
                GetNextToken(compilerInfo, &parseInfo.next_token);
                subtree->child[2] = _stmtseq();
            }
        }

        if (parseInfo.next_token.type != END) {
            throw 0;
        }
        GetNextToken(compilerInfo, &parseInfo.next_token);


        return subtree;
    }

    TreeNode *_readstmt() {
        TreeNode *leafNode = new TreeNode;
        leafNode->line_num = compilerInfo->in_file.cur_line_num;
        leafNode->node_kind = READ_NODE;

        GetNextToken(compilerInfo, &parseInfo.next_token);
        if (parseInfo.next_token.type == ID) {
            AllocateAndCopy(&leafNode->id, parseInfo.next_token.str);
            GetNextToken(compilerInfo, &parseInfo.next_token);
        } else {
            throw 0;
        }
        return leafNode;
    }

    TreeNode *_repeatstmt() {
        TreeNode *tree = new TreeNode;
        TokenType tokenType = parseInfo.next_token.type;

        tree->node_kind = REPEAT_NODE;
        tree->line_num = compilerInfo->in_file.cur_line_num;

        if (tokenType == REPEAT) {
            AllocateAndCopy(&tree->id, parseInfo.next_token.str);
            GetNextToken(compilerInfo, &parseInfo.next_token);
            tree->child[0] = _stmtseq();
            tokenType = parseInfo.next_token.type;
        }
        if (tokenType == UNTIL) {
            GetNextToken(compilerInfo, &parseInfo.next_token);
            AllocateAndCopy(&tree->id, parseInfo.next_token.str);
            //GetNextToken(compilerInfo, &parseInfo.next_token);
            tree->child[1] = _expr();
        } else {
            throw 0;
        }

        return tree;
    }

    TreeNode *_assignstmt() {
        TreeNode *subtree = new TreeNode;
        subtree->line_num = compilerInfo->in_file.cur_line_num;
        subtree->node_kind = ASSIGN_NODE;


        // first i get the id second get the := third move to _expr to evaluate the last part
        // id := _expr

        if (parseInfo.next_token.type == ID) {
            AllocateAndCopy(&subtree->id, parseInfo.next_token.str);
        } else {
            // ERROR as we cant do 1 := 7
            throw 0;
        }
        // Now get the :=
        GetNextToken(compilerInfo, &parseInfo.next_token);
        //Now get the _expr
        GetNextToken(compilerInfo, &parseInfo.next_token);
        subtree->child[0] = _expr();
        return subtree;
    }

    TreeNode *_writestmt() {
        TreeNode *writeNode = new TreeNode;
        writeNode->node_kind = WRITE_NODE;
        writeNode->line_num = compilerInfo->in_file.cur_line_num;


        GetNextToken(compilerInfo, &parseInfo.next_token);
        writeNode->child[0] = _expr();

        return writeNode;
    }

    TreeNode *_expr() {
        TreeNode *subtree;
        subtree = _mathexpr();

        TreeNode *operationNode = new TreeNode;
        TokenType operation = parseInfo.next_token.type;

        if (operation == LESS_THAN || operation == EQUAL) {
            operationNode->node_kind = OPER_NODE;
            operationNode->oper = operation;
            operationNode->expr_data_type = BOOLEAN;

            //Declare the operation node as the parent of two sides mathexprs
            GetNextToken(compilerInfo, &parseInfo.next_token); //Get the 2nd mathexpr
            operationNode->child[0] = subtree;
            operationNode->child[1] = _mathexpr(); //2nd mathexpr subtree

            return operationNode;
        } else {
            return subtree;
        }
    }

    TreeNode *_mathexpr() {
        //SAME AS IN EXPR WE TEST FOR OPERATION AND ASSIGN IF PRESENT
        TreeNode *subtree;
        subtree = _term();

        //BUT WE HAVE 0,1, OR MORE + - Operations
        while (parseInfo.next_token.type == PLUS || parseInfo.next_token.type == MINUS) {
            TreeNode *operation_tree = new TreeNode;

            operation_tree->oper = parseInfo.next_token.type;
            operation_tree->node_kind = OPER_NODE;
            operation_tree->expr_data_type = INTEGER;
            operation_tree->line_num = compilerInfo->in_file.cur_line_num;

            operation_tree->child[0] = subtree;

            GetNextToken(compilerInfo, &parseInfo.next_token);
            operation_tree->child[1] = _term();

            subtree = operation_tree;


        }
        return subtree;
    }

    TreeNode *_term() {
        TreeNode *subtree = _factor();

        while (parseInfo.next_token.type == TIMES || parseInfo.next_token.type == DIVIDE) {
            TreeNode *operation_tree = new TreeNode;
            operation_tree->node_kind = OPER_NODE;

            operation_tree->line_num = compilerInfo->in_file.cur_line_num;
            operation_tree->oper = parseInfo.next_token.type;

            operation_tree->child[0] = subtree;

            GetNextToken(compilerInfo, &parseInfo.next_token);
            operation_tree->child[1] = _factor();

            subtree = operation_tree;
        }
        return subtree;
    }

    TreeNode *_factor() {
        TreeNode *subtree = _newexpr();

        while (parseInfo.next_token.type == POWER) {
            TreeNode *operation_tree = new TreeNode;
            operation_tree->node_kind = OPER_NODE;
            operation_tree->line_num = compilerInfo->in_file.cur_line_num;
            operation_tree->oper = parseInfo.next_token.type;
            operation_tree->child[0] = subtree;

            GetNextToken(compilerInfo, &parseInfo.next_token);
            operation_tree->child[1] = _factor();

            subtree = operation_tree;
        }
        return subtree;
    }

    TreeNode *_newexpr() {
        TokenType tokenType = parseInfo.next_token.type;
        TreeNode *subtree = new TreeNode;
        subtree->line_num = compilerInfo->in_file.cur_line_num;

        if (tokenType == NUM) {
            subtree->node_kind = NUM_NODE;
            subtree->num = stoi(parseInfo.next_token.str);
            subtree->expr_data_type = INTEGER;
            //subtree->expr_data_type = INTEGER;
        } else if (tokenType == ID) {
            subtree->node_kind = ID_NODE;
            //subtree->id = parseInfo.next_token.str;
            AllocateAndCopy(&subtree->id, parseInfo.next_token.str);
            subtree->expr_data_type = INTEGER;
        } else if (tokenType == LEFT_PAREN) { // TODO

            GetNextToken(compilerInfo, &parseInfo.next_token);
            TreeNode *expr_tree = _mathexpr();
            // THIS SKIPS THE RIGH_PAREN
            GetNextToken(compilerInfo, &parseInfo.next_token);

            return expr_tree;
        } else {
            throw 0;
        }

        GetNextToken(compilerInfo, &parseInfo.next_token);
        return subtree;
    }


    ~Parser() {
        delete parseTreeRoot;
    }

};


void start(CompilerInfo& compilerInfo){
    //Scanner(&compilerInfo);

    Parser parser(&compilerInfo);
    TreeNode *parseTree = parser.Parse();
    if (parseTree != nullptr) {
        PrintTree(parseTree);
    }
}

int main() {
    char *filename = new char[500];
    cin >> filename;
    CompilerInfo compilerInfo(filename, "output.txt", "debug.txt");
    start(compilerInfo);
    return 0;
}
