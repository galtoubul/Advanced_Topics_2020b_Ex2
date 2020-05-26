#include <filesystem>
namespace fs = std::filesystem;
class Travel {
    int index;
    fs::path shipPlanPath;
    fs::path shipRoutePath;
    fs::path dir;

public:
    Travel(int _index, fs::path _shipPlanPath, fs::path _shipRoutePath, fs::path _dir) :
    index(_index), shipPlanPath(_shipPlanPath), shipRoutePath(_shipRoutePath), dir(_dir) {}

    int getIndex();

    fs::path& getShipPlanPath();

    fs::path& getShipRoutePath();

    fs::path& getDir();
};

