/**
 * ErrorsInterface Summary:
 * This module contains all program errors and messages to be printed.
 */

#include <map>

using std::string;
using std::to_string;

#define UNABLE_TO_OPEN_FILE(fileName)                       cout << "ERROR: Unable to open file: " << fileName << " Exiting..." << endl;

#define CONTAINER_ERROR(x)                                  cout << "ERROR: Conatiner's " << x << " input isn't valid. Exiting..." << endl;

#define INVALID_INPUT(x)                                    cout << "ERROR: " << x << "input isn't valid. Exiting..." << endl;

#define PORT_FILE_NAME_ISNT_MATCHING(fileName)              cout << "ERROR: There isn't any port file name matching " << fileName << " .Exiting..." << endl;

#define NON_LEGAL_SEA_PORT_CODE(port)                       cout << "ERROR: Ship route input isn't valid. It contains a line with non legal seaport code: " << port << ". Exiting..." << endl;
#define SAME_PORT_AS_PREV                                   cout << "ERROR: Ship route input isn't valid. It contains the same port in two consecutive lines. Exiting..." << endl;

#define NOT_LEGAL_OPERATION(C, id, floor, x, y, reason)     cout << "ERROR: " << C" " << id << " at floor: " << floor << " at indices: [" << x << "][" << y << "] isn't legal\nThe reason: " << reason << "\nLeaving current travel..." << endl;
#define CONTAINER_FORGOTTEN(currPortSymbol)                 cout << "ERROR: There is a Container whose destination isn't" << currPortSymbol << " and yet it was Forgotten there.\nLeaving current travel..." << endl;
#define CONTAINER_WASNT_DROPPED(currPortSymbol)             cout << "ERROR: There is a Container whose destination isn't" << currPortSymbol << "and yet it was Forgotten on ship.\nLeaving current travel..." << endl;

#define LAST_PORT_WARNING                                   cout << "Warning: This is the last port at the ship a route, but it has containers to unload.\nAll of these containers won't be unloaded from port." << endl;

#define TRAVELS_OR_ALGORITHMS_NUMBER_ERROR(x)               std::cout << "Number of " << x << " has to be positive." << std::endl;

class ErrorsInterface {
public:
    static std::map<int, std::string> errorsMap;

    static void populateErrorsMap() {
        errorsMap.insert({(1 << 0),
                          " ship plan: a position has an equal number of floors, or more, than the number of floors provided in the first line"});
        errorsMap.insert({(1 << 1), "ship plan:a given position exceeds the X/Y ship limits"});
        errorsMap.insert({(1 << 2),
                          "ship plan: bad line format after first line or duplicate x,y appearance with same data"});
        errorsMap.insert({(1 << 3),
                          "ship plan: travel error - bad first line or file cannot be read altogether - cannot run this travel"});
        errorsMap.insert({(1 << 4),
                          "ship plan: travel error - duplicate x,y appearance with different data - cannot run this travel"});
        errorsMap.insert({(1 << 5), "travel route: a port appears twice or more consecutively"});
        errorsMap.insert({(1 << 6), "travel route: bad port symbol format"});
        errorsMap.insert({(1 << 7),
                          "travel route: travel error - empty file or file cannot be read altogether - cannot run this travel"});
        errorsMap.insert({(1 << 8),
                          "travel route: travel error - file with only a single valid port - cannot run this travel"});
        errorsMap.insert({(1 << 9), "reserved"});
        errorsMap.insert({(1 << 10), "containers at port: duplicate ID on port"});
        errorsMap.insert({(1 << 11), "containers at port: ID already on ship"});
        errorsMap.insert({(1 << 12), "containers at port: bad line format, missing or bad weight"});
        errorsMap.insert({(1 << 13), "containers at port: bad line format, missing or bad port dest"});
        errorsMap.insert({(1 << 14), "containers at port: bad line format, ID cannot be read"});
        errorsMap.insert({(1 << 15), "containers at port: illegal ID check ISO 6346"});
        errorsMap.insert({(1 << 16), "containers at port: file cannot be read altogether"});
        errorsMap.insert({(1 << 17), "containers at port: last port has waiting containers"});
        errorsMap.insert({(1 << 18), "containers at port: total containers amount exceeds ship capacity"});
    }

    static string buildNotLegalOperationError(string op, string id, int floor, int x, int y, string reason) {
        return "ERROR: " + op + " " + id + " at floor: " + to_string(floor) + " at indices: [" + to_string(x) + "][" +
               to_string(y) + "] isn't legal\nThe reason: " + reason + "\nLeaving current travel...";
    }

    static string buildContainerForgottenError(string currPortSymbol) {
        return "ERROR: There is a Container whose destination isn't" + currPortSymbol +
               " and yet it was Forgotten there.\nLeaving current travel...";
    }

    static string buildContainerWasntDroppedError(string currPortSymbol) {
        return "ERROR: There is a Container whose destination isn't" + currPortSymbol +
               " and yet it was Forgotten on ship.\nLeaving current travel...";
    }
};