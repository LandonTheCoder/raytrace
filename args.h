// Used by the calling program
struct args {
    /* Index of filename.
     * On Windows, the OS sometimes does lossy argument conversion from UTF-16
     * to Active Code Page, and the argument may need to be converted from
     * UTF-16 again. This aids for that.
     */
    int fname_pos;
    // Enum value of file type (Default: PPM for stdout, extension-determined for file)
    int ftype;
    // File name (or nullptr if stdout)
    char *fname;
};

// Parses args into a format that can more easily be used.
struct args parse_args(int argl, char **args);