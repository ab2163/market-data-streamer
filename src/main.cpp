#include <iostream>
#include <databento/historical.hpp>

using namespace std;
using namespace databento;

int main(int argc, char *argv[]){
    if(argc < 2){
        cerr << "Path to DBN file missing.\n";
        return 1;
    }

    string filepath = argv[1];
    auto file_store = DbnFileStore(filepath);
    cout << file_store.GetMetadata() << '\n';
    const Record *record = file_store.NextRecord();
    cout << "Size: " << record->Size() << endl;
    const auto& mbo_msg = record->Get<MboMsg>();
    cout << "Price: " << mbo_msg.price << endl;
    cout << "Size: " << mbo_msg.size << endl;


    return 0;
}