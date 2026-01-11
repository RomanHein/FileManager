# -- Introduction --
### What does the FileManager do? 
The manager is essentially just a class which **handles all the verbose code parts of working with files in C++17**.
The class provides user-friendly methods for file manipulation and makes the developer's work easier by freeing them of the concerns that come with file management (saving changes, reading specific rows, bound checking, etc.).

### Where does it shine?
The file manager is lightweight and easy to use, it lies somewhere in between of raw file I/O and a professional database.
Some areas where it can be useful area:
- Simple list to keep track of scores.
- Vocabulary trainer.

# -- Features --
- File content is **loaded into RAM** for performance.
- Creates **recovery files in case of a failed save**.
- **Keeps memory low** by cleaning garbage when necessary.
- **Saves efficiently** by evaluating whether a full rewrite is necessary.
- **Easy to use** due to idiomatic methods with additional comments.
- Standalone, **independent library** which can just be dropped into the project folder.

# -- Code samples --
```
// The manager is a header only library.
#include "file_manager.h"

// Creates a test.txt file at the root of the C drive, depending on the provided path.
// If the given path/file doesn't exist, the manager creates the according directories/file.
FileManager fm(R"C:/test.txt");

// Appends "Hello world!" at the end of the txt file.
fm.append("Hello world", "!");

// Optional: save changes. Happens automatically when the destructor is called.
fm.save();
```

# -- Class Methods --
| Method  | Explanation |
|---------|-------------|
| FileManager(filePath) | Creates a new FileManager instance that manages the specified file. |
| read(row) | Returns the text at the specified row. |
| split(row, delimiter) | Returns the text parts of split text at the specified row by the specified delimiter. |
| first() | Returns a copy of the text at the first row. |
| last() | Returns a copy of the text at the last row. |
| all() | Returns a copy of the text at every row. |
| append(args) | Adds the given arguments to a new row at the end of the file. |
| overwrite(row, args) | Overwrites the specified row with the specified arguments |
| erase(row) | Deletes the specified row, shifting all later elements down. |
| clear() | Deletes all rows. |
| save() | Saves all changes back to the file. |
| empty() | Returns true if there are no present rows. |
| size() | Returns the number of present rows. |
