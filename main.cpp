#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <filesystem>

using namespace std;
namespace fs = filesystem;

const fs::path saveDir = "savedNotes"; // Directory that notes are saved to.
const string noteExt = ".cppn"; // Extension that notes are saved with.
const string headSep = " | "; // Seperator used in the head of a note.

// Command used to clear screen is platform-dependent.
#if defined(_WIN32) || defined(_WIN64)
    const char* clearScreen = "cls";
#else
    const char* clearScreen = "clear";
#endif

/// Represents a note that can be created by the user.
/// 
/// Attributes:
/// - 'id': The identification number of the note.
/// - 'name': The name of the note.
/// - 'timestamp': The time that the note was created.
/// - 'content': The content of the note.
class Note {
    private:
        string name;
        string timestamp;
        string content = "";
    
    public:
        // Constructor
        Note(string nameVal, string timestampVal, string contentVal) {
            name = nameVal;
            timestamp = timestampVal;
            content = contentVal;
        }

        // Getter methods.
        string getName() const { return name; }
        string getTimestamp() const { return timestamp; }
        string getContent() const { return content; }

        // Setter methods.
        void setName(string newName) { name = newName; }
        void setTimestamp(string newTimestamp) { timestamp = newTimestamp; }
        void setContent(string newContent) { content = newContent; }
};

/// Grabs the local computer's current time and displays it in a nice format.
///
/// Returns a user-friendly string representing the local computer's current
/// time.
string getCurrentTime() {
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);
    tm local_tm;

#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&local_tm, &now_time);
#else
    localtime_r(&now_time, &local_tm);
#endif

    stringstream ss;
    ss << put_time(&local_tm, "%Y-%m-%d [%H:%M]");
    return ss.str();
}

/// Counts the number of words in a string of text.
///
/// Returns the number of words in <text>.
///
/// Args:
/// - 'text': The text that is being counted.
int countWords(const string& text) {
    istringstream stream(text);
    string word;
    int count = 0;

    while (stream >> word) {
        count++;
    }

    return count;
}

/// Extracts the argument from a valid command.
///
/// Returns the argument found in <text>.
///
/// Args:
/// - 'text': The command that contains an argument.
string extractArg(const string& text) {
    if (text.find(" ") != string::npos) {
        const size_t firstSpacePos = text.find(" ");
        return text.substr(firstSpacePos + 1);
    }

    return "";
}

/// Checks if <text> contains any characters from <chars>.
///
/// Returns true if <text> contains any characters from <chars>, false
/// otherwise.
///
/// Args:
/// - 'text': The text that is being checked.
/// - 'chars': A string of chars that is being checked against <text>.
bool containsCharsFrom(const string& text, const string& chars) {
    for (char c : text) {
        if (chars.find(c) != string::npos) {
            return true;
        }
    }

    return false;
}

/// Checks if <input> is a valid filename.
///
/// Returns true if <input> is a valid filename, false otherwise.
///
/// Args:
/// - 'input': The user input that is being validated.
bool validateInput(const string& input) {
    const string invalidChars = "<>:\"/\\|?*";
    const int maxLength = 255;

    if (containsCharsFrom(input, invalidChars)) {
        return false;
    } else if (input.length() >= 255) {
        return false;
    }

    return true;
}

/// Saves a given note to the current directory.
///
/// Args:
/// - 'note': The note that is being saved.
void saveNote(const Note& note) {
    const auto filePath = saveDir / (note.getName() + noteExt);
    ofstream outfile(filePath);

    if (outfile.is_open()) {
        outfile << note.getContent();
        outfile.close();
        cout << note.getName() << " successfully saved!\n\n";
    } else {
        cout << "ERROR: " << note.getName() << " failed to save.\n\n";
    }
}

/// Handles the editing of a note.
///
/// Args:
/// - 'note': The note that is being opened.
void openNote(Note& note) {
    string line;
    string newContent;
    const size_t startUserContent = note.getContent().find("\n\n");
    const string userContent = note.getContent().substr(startUserContent + 2);
    const string head = note.getName() + headSep + note.getTimestamp() + "\n\n";

    system(clearScreen);
    cout << "" << note.getName() << headSep << note.getTimestamp() << "\n";
    cout << "Type !quit on a new line to exit.\n\n";
    cout << userContent;

    while (true) {
        getline(cin, line);
        if (line == "!quit") break;
        newContent += line + "\n";
    }

    note.setContent(head + userContent + newContent);

    saveNote(note);
}

/// Creates a new note and opens it.
///
/// Args:
/// - 'title': The given name of the new note.
void createNote(const string& title) {
    if (fs::exists(saveDir / (title + noteExt))) {
        cout << "ERROR: '" << title << "' already exists.\n\n";
    } else {
        Note note(title, getCurrentTime(), "");
        note.setContent(note.getName() + headSep + note.getTimestamp() + "\n\n");
        openNote(note);
    }
}

/// Loads a note from the current directory.
///
/// Args:
/// - 'title': The name of the requested note.
/// - 'appendMode': True if user is appending, false if user is overwriting.
void loadNote(const string& title, const bool& appendMode) {
    const auto filePath = saveDir / (title + noteExt);
    ifstream infile(filePath);
    string head;
    string line;
    string loadedContent;

    if (infile.is_open()) {
        system(clearScreen);
        getline(infile, head);

        size_t sep = head.find(headSep);
        Note note(title, head.substr(sep + headSep.length()), "");
        
        if (appendMode) {
            for (int i = 1; getline(infile, line); ++i) {
                loadedContent += line + "\n";
            }
        } else {
            loadedContent = "\n";
        }

        note.setContent(head + "\n" + loadedContent);
        openNote(note);

    } else {
        cout << "ERROR: '" << title << "' does not exist or "
                "failed to load.\n\n";
    }
}

/// Prints a list of all saved notes to the user.
void listNotes() {
    const auto dirIter = fs::directory_iterator(saveDir);

    if (!fs::exists(saveDir) || !fs::is_directory(saveDir)) {
        cout << "ERROR: Could not find save directory.\n\n";
        return;
    }

    if (dirIter == fs::directory_iterator{}) {
        cout << "No files found.\n\n";
        return;
    }

    for (const auto& entry : dirIter) {
        auto path = entry.path();
        cout << path.stem().string() << "\n";
    }

    cout << "\n";
}

/// Deletes the note with the given name.
///
/// Args:
/// - 'title': The name of the note that the user wants to delete.
void deleteNote(const string& title) {
    const auto filePath = saveDir / (title + noteExt);

    if (fs::remove(filePath)) {
        cout << title << " successfully deleted!\n\n";
    } else {
        cout << "ERROR: " << title << " not found or failed to delete.\n\n";
    }
}

/// Handler function for the user commands and prompts.
void promptHandler() {
    string cmd;

    // User loop.
    while (true) {
        cout << "$~ ";
        getline(cin, cmd);
        const string arg = extractArg(cmd);

        if (cmd == "exit") {
            break;

        } else if (cmd == "help") {
            cout << "- 'new [note]' to create a new note.\n"
                    "- 'app [note]' to append an existing note.\n"
                    "- 'ow [note]' to overwrite an existing note.\n"
                    "- 'del [note]' to delete an existing note.\n"
                    "- 'cls' to clear the screen.\n"
                    "- 'exit' to exit the program.\n\n";
        
        } else if (cmd == "cls") {
            system(clearScreen);

        } else if (cmd == "list") {
            listNotes();
        
        // Any conditions after these require valid input for filenames.
        } else if (countWords(cmd) == 2 && !validateInput(arg)) {
            cout << "'" << arg << "' is not a valid filename.\n\n";
        
        } else if (cmd.compare(0, 4, "del ") == 0 && countWords(cmd) == 2) {
            deleteNote(arg);

        } else if (cmd.compare(0, 4, "new ") == 0 && countWords(cmd) == 2) {
            createNote(arg);

        } else if (cmd.compare(0, 4, "app ") == 0 && countWords(cmd) == 2) {
            loadNote(arg, true);
        
        } else if (cmd.compare(0, 3, "ow ") == 0 && countWords(cmd) == 2) {
            loadNote(arg, false);

        } else if (cmd.compare(0, 3, "del") == 0 ||
                   cmd.compare(0, 3, "new") == 0 ||
                   cmd.compare(0, 3, "app") == 0 ||
                   cmd.compare(0, 2, "ow") == 0) {
            cout << "ERROR: Missing argument (filename).\n\n";
            
        } else {
            cout << "'" << cmd << "' is not a valid command.\n\n";
        }
    }
}

/// CPPNotes is a barebones console notes program that allows the user to
/// create and load notes through their terminal.
int main() {
    cout << "Welcome to CPPNotes!\n";
    cout << "Enter a command (new | app | ow | list | del | help | cls | "
            "exit)\n\n";

    // Make sure the save directory 'savedNotes\' always exists.
    if (!fs::exists(saveDir)) {
        fs::create_directories(saveDir);
    }
    
    promptHandler();

    return 0;
}
