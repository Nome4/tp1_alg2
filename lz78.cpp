#include<iostream>
#include<fstream>
#include<functional>
#include<map>
#include<cmath>
#include<vector>
#include<unistd.h>
#define buffer_size 65536
using namespace std;

struct PrefixTree{
    map<char, PrefixTree *> children;
    int ind;

    PrefixTree(int ind2){
        ind = ind2;
    }

    ~PrefixTree(){
        for (const auto &p : children){
            delete p.second;
        } 
    }
};

struct WriteBuffer{
    char *buffer;
    ostream *out;
    int pos;

    WriteBuffer(char *b,ostream *o){
        buffer=b;
        out=o;
        pos=0;
    }

    void write_byte(char c){
        if(pos==buffer_size){
            pos=0;
            out->write(buffer,buffer_size);
        }
        buffer[pos]=c;
        pos++;
    }

    void close(){
        out->write(buffer,pos);
    }
};

int calc_nbytes(int n){
    // cout<<n<<"\n";
    int ret=0;
    while(n>0){
        n>>=8;
        ret++;
    };
    return ret;
}

void comprimir(istream &in, ofstream &out){
    PrefixTree *root=new PrefixTree(0);
    PrefixTree *curr=root;
    int ind=0;
    char read_buf[buffer_size];
    char aux_buf[buffer_size];
    auto write_buf=WriteBuffer(aux_buf,&out);

    auto write_ind=[&](int x){
        int nbytes=calc_nbytes(ind);
        for(int i=0; i<nbytes; i++){
            write_buf.write_byte((char)(x>>8*(nbytes-i-1)));
        }
    };

    while(true){
        in.read(read_buf,buffer_size);
        int num_read=in.gcount();
        if(num_read==0){
            break;
        }

        for(int i=0; i<num_read; i++){
            auto it=curr->children.find(read_buf[i]);
            if(it==curr->children.end()){
                if(ind!=0){
                    write_ind(curr->ind);
                }
                ind++;
                PrefixTree *n2=new PrefixTree(ind);
                curr->children[read_buf[i]]=n2;
                write_buf.write_byte(read_buf[i]);
                curr=root;
            }
            else{
                curr=it->second;
            }
        }
    }

    write_ind(curr->ind);
    delete root;
    write_buf.close();
}

struct InvertedTree{
    int parent;
    char c;

    InvertedTree(int p,char c2){
        parent=p;
        c=c2;
    }
};

void descomprimir(istream &in, ofstream &out){
    vector<InvertedTree> nodes;
    nodes.push_back(InvertedTree(-1,'\0'));
    char read_buf[buffer_size];
    char aux_buf[buffer_size];
    auto write_buf=WriteBuffer(aux_buf,&out);
    int pos=0,num_read=0;
    int parent=0;

    auto read_byte=[&](){
        if(pos==num_read){
            in.read(read_buf,buffer_size);
            num_read=in.gcount();
            if(num_read==0){
                return '\0';
            }
            pos=0;
        }
        char c=read_buf[pos];
        pos++;
        return c;
    };

    while(true){
        char c=read_byte();
        if(num_read==0){
            break;
        }

        int ind=0;
        int nbytes=calc_nbytes(nodes.size());
        nodes.push_back(InvertedTree(parent,c));
        write_buf.write_byte(c);

        for(int i=0; i<nbytes; i++){
            char c=read_byte();
            if(num_read==0){
                goto end;
            }
            ind|=(unsigned char)c<<8*(nbytes-i-1);
        }

        string decoded="";
        parent=ind;
        while(ind!=0){
            decoded+=nodes[ind].c;
            ind=nodes[ind].parent;
        }
        for(int i=decoded.length()-1; i>=0; i--){
            write_buf.write_byte(decoded[i]);
        }
    }

    end:;
    write_buf.close();
}

int main(int argc, char **argv){
    string arq_in="", arq_out="";
    int c;
    int modo=-1; // 0 = comprimir, 1 = descomprimir
    while ((c = getopt(argc, argv, "c:o:x:")) != -1){
        switch (c){
            case 'c':
                arq_in = optarg;
                modo=0;
                break;
            case 'o':
                arq_out = optarg;
                break;
            case 'x':
                arq_in = optarg;
                modo=1;
                break;
        }
    }
    
    if(arq_in==""){
        cout<<"Informe ao menos o arquivo de entrada\n";
        cout<<"Formato certo: ./lz78 <-c ou -x> <arquivo_entrada> [-o <arquivo_saida>]\n";
        return 0;
    }
    if(arq_out==""){
        int pos=arq_in.find_last_of('.');
        if(pos==string::npos){
            pos=arq_in.length();
        }
        arq_out=arq_in.substr(0,pos)+(modo==0? ".z78": ".txt");
    }

    ifstream in;
    in.open(arq_in,ios_base::binary);
    ofstream out;
    out.open(arq_out,ios_base::binary);

    if(!in.is_open()){
        cout<<"Arquivo de entrada inexistente ou indisponível\n";
        return 0;
    }
    if(!out.is_open()){
        cout<<"Arquivo de saída inexistente ou indisponível\n";
        return 0;
    }

    if(modo==0){
        comprimir(in,out);
    }
    else{
        descomprimir(in,out);
    }
    return 0;
}
