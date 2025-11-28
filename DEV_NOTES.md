# Development Notes

## Build System (`make`)
- **Incremental Build (`make`)**: Compiles only modified source files. Use this for standard development.
- **Full Rebuild (`make clean && make`)**: Forces recompilation of all files. Use this when:
    - Modifying header files (`.h`) that might not be correctly tracked.
    - Renaming or deleting source files.
    - Changing compiler flags.
    - Encountering unexplained linker errors.

## Troubleshooting
### Deadlock in `SessionManager`
- **Issue**: `get_user_id_by_fd` locked `mutex_` and then called `get_user_id_by_session`, which attempted to lock `mutex_` again. Since `std::mutex` is not recursive, this caused a deadlock.
- **Fix**: Modified `get_user_id_by_fd` to access the internal maps (`fd_to_session_id_` and `sessions_`) directly after locking, avoiding the nested method call.


git show main:run.sh > old_run.sh