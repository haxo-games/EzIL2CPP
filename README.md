# EzIL2CPP

## What is this?

A single-header C++ helper library to facilitate interaction with the IL2CPP API. Like many of our public projects this was made because we needed it to better explain some of our courses.

## Usage

### `Resolver`

`Resolver` is a class you can instantiate and initialize through its constructor by passing the address of the `GameAssembly.dll` binary. Under the hood it contains a bunch of function pointers which it initializes 
by walking through the exports within this same module. You can then call these IL2CPP API functions by simply invoking them by their name (they are named identically to their export).
<br />
<br />
On top of the "imported" IL2CPP API functions the class contains some useful utilities:
- `getAssemblyNames(void* p_domain)`: Returns a list of strings of all the assemblies.
- `getClassFields(void* p_klass)`: Returns a list of `CustomFieldInfo` for the provided class.
- `getClassMethods(void* p_klass)`: Returns a list of `CustomMethodInfo` for the provided class.
- `getAssemblyImageClasses(Image* p_image)`: Returns a list of `CustomClassInfo` for the provided assembly image.
- `jsonifyRuntimeData(void* p_domain)`: Returns a JSON-formatted string containing all the different assemblies and their classes with their methods and fields. Useful if you're dealing with a game that obfuscates
its global metadata file.
- `getErrorMessage()`: If anything goes wrong anywhere an error message string is set which you can read by getting its reference from this method.
- `getHasError()`: Indicates whether something went wrong or not.

## Documentation

There isn't really any documentation regarding the IL2CPP API functions so I decided it could be useful to document them as best as we can here.

### `il2cpp_domain_get()`

Returns the value of the global pointer to the domain object. If said pointer is 0 then it initializes the domain.

### `il2cpp_domain_get_assemblies(void* p_domain, uint64_t* p_assembly_count)`

Takes in both the target domain (does nothing with it) and a pointer to an 8 bytes location to write the amount of assemblies to. To calculate the size
it subtracts the global point to the end of the assemblies list from the one to the start of it. Finally it returns the pointer to the start of the assemblies.

### `il2cpp_domain_assembly_open(void* p_domain, const char* assembly_name)`

Takes in both the target domain (does nothing with it) and the target assembly's name. Under the hood it simply goes through the list of assemblies and compares each of
their names with the one provided until there is a match, in which case it would return the pointer to the matched assembly.

### `il2cpp_assembly_get_image(void* p_assembly)`

Takes in the pointer to an assembly object and returns the pointer the image object it contains.

## Contributing

This project is free and open source and will remain so forever. You are welcome to contribute. Simply make a pull request for whatever it is you would like to add, but
there are a few things you need to keep in mind:

1. C++17 only for now.
2. snake_case for variable names, PascalCase for namespaces and structures and camelCase for methods and function names (there might be more specifics so please just
base your style on the already existing code).
3. Make an issue describing your feature or bugfix and specify your intent to address said issue. Once you make the pull request you can auto-close the issue by doing `close #<issue_number_here>`.
4. When making a branch give it a meaningful name like `feature/name_of_feature` or `fix/name_of_fix`.

## License

This project is licensed under the MIT License with some specifications - see the [LICENSE](LICENSE) file for details.

