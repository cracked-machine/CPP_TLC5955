## Running Units Tests on X86

When you run the default CMake build, the output is linked with the Catch2 library. To run the testsuite use the command:
`./build/test_suite`

See `.vscode/tasks.json` for details on the individual toolchain commands.

Note, this project uses the mock cmsis defined in the [embedded_utils](https://github.com/cracked-machine/embedded_utils/tree/main/tests) repo.