//  ______     _____ _      ___   _____ _____  _____  
// |  ____|   |_   _| |    |__ \ / ____|  __ \|  __ \ 
// | |__   ____ | | | |       ) | |    | |__) | |__) |
// |  __| |_  / | | | |      / /| |    |  ___/|  ___/   Single-header C++ helper to interact with the IL2CPP API
// | |____ / / _| |_| |____ / /_| |____| |    | |       version 0.0.0
// |______/___|_____|______|____|\_____|_|    |_|       https://github.com/haxo-games/EzIL2CPP
//
// SPDX-FileCopyrightText: 2024 - 2025 Haxo Games Inc. <https://haxo.games>
// SPDX-License-Identifier: MIT

#pragma once

#include <windows.h>

class EzIL2CPP
{
public:
	void* (*il2cpp_domain_get)();
	void* (*il2cpp_domain_assembly_open)(void* p_domain, const char* name);
	void* (*il2cpp_assembly_get_image)(void* p_assembly);
	void* (*il2cpp_thread_attach)(void* p_domain);
	void* (*il2cpp_runtime_invoke)(void* p_method, void* p_object, void** params, void** exceptions);
	void* (*il2cpp_class_from_name)(void* p_image, const char* namespaze, const char* name);
	void* (*il2cpp_class_get_type)(void* p_class);
	void* (*il2cpp_type_get_object)(void* p_type);
	void* (*il2cpp_class_get_method_from_name)(void* p_class, const char* name, int args_count);
	void* (*il2cpp_class_get_field_from_name)(void* p_class, const char* iter);
	void* (*il2cpp_field_get_value)(void* p_obj, void* p_field, void* p_value);

	EzIL2CPP(HMODULE h_game_assembly)
	{
		initialize(h_game_assembly);
	}

	bool initialize(HMODULE h_game_assembly)
	{
		if (is_initialized)
			return true;

		if (h_game_assembly == 0)
		{
			setError("initialize(): h_game_assembly was NULL");
			return false;
		}

		this->h_game_assembly = h_game_assembly;

		if (!resolveImports())
			return false;

		is_initialized = true;
		return true;
	}

	const std::string& getErrorMessage() const
	{
		return error_message;
	}

	bool getHasError() const
	{
		return has_error;
	}

	bool getIsInitialized() const
	{
		return is_initialized;
	}

	void setError(std::string fmt, ...)
	{
		/* Clear error if empty message */
		if (fmt.size() == 0)
		{
			has_error = false;
			error_message.clear();
			return;
		}

		va_list args;
		va_start(args, fmt);

		char* buf{ new char[0x1000] };
		vsprintf_s(buf, 0x1000, fmt.c_str(), args);
		error_message = buf;

		delete[] buf;
		va_end(args);
		has_error = true;
	}

private:
	bool is_initialized{};
	std::string error_message;
	bool has_error{};
	HMODULE h_game_assembly;

	void* resolveSingleImport(IMAGE_EXPORT_DIRECTORY* p_export_directory, const char* target_export)
	{
		static DWORD* address_table{ reinterpret_cast<DWORD*>(reinterpret_cast<uintptr_t>(h_game_assembly) + p_export_directory->AddressOfFunctions) };
		static DWORD* name_table{ reinterpret_cast<DWORD*>(reinterpret_cast<uintptr_t>(h_game_assembly) + p_export_directory->AddressOfNames) };
		static WORD* ordinal_table{ reinterpret_cast<WORD*>(reinterpret_cast<uintptr_t>(h_game_assembly) + p_export_directory->AddressOfNameOrdinals) };

		for (size_t i{}; i < p_export_directory->NumberOfNames; i++)
		{
			const char* export_name{ reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(h_game_assembly) + name_table[i]) };

			if (strcmp(export_name, target_export) == 0)
				return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(h_game_assembly) + address_table[ordinal_table[i]]);
		}

		return nullptr;
	}

	bool resolveImports()
	{
		IMAGE_DOS_HEADER* p_dos_header{ reinterpret_cast<IMAGE_DOS_HEADER*>(h_game_assembly) };

		if (p_dos_header->e_magic != IMAGE_DOS_SIGNATURE)
		{
			setError("resolveImports(): DOS header magic is invalid");
			return false;
		}

		IMAGE_NT_HEADERS* p_nt_headers{ reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uintptr_t>(h_game_assembly) + p_dos_header->e_lfanew) };

		if (p_nt_headers->Signature != IMAGE_NT_SIGNATURE)
		{
			setError("resolveImports(): NT signature is invalid");
			return false;
		}

		if (p_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size == 0)
		{
			setError("resolveImports(): The export directory of the game assembly is marked with no entries");
			return false;
		}

		IMAGE_EXPORT_DIRECTORY* p_export_directory{ reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(reinterpret_cast<uintptr_t>(h_game_assembly) + p_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress) };
		il2cpp_domain_get = reinterpret_cast<void* (*)()>(resolveSingleImport(p_export_directory, "il2cpp_domain_get"));
		il2cpp_domain_assembly_open = reinterpret_cast<void* (*)(void*, const char*)>(resolveSingleImport(p_export_directory, "il2cpp_domain_assembly_open"));
		il2cpp_assembly_get_image = reinterpret_cast<void* (*)(void*)>(resolveSingleImport(p_export_directory, "il2cpp_assembly_get_image"));
		il2cpp_thread_attach = reinterpret_cast<void* (*)(void*)>(resolveSingleImport(p_export_directory, "il2cpp_thread_attach"));
		il2cpp_runtime_invoke = reinterpret_cast<void* (*)(void*, void*, void**, void**)>(resolveSingleImport(p_export_directory, "il2cpp_runtime_invoke"));
		il2cpp_class_from_name = reinterpret_cast<void* (*)(void*, const char*, const char*)>(resolveSingleImport(p_export_directory, "il2cpp_class_from_name"));
		il2cpp_class_get_type = reinterpret_cast<void* (*)(void*)>(resolveSingleImport(p_export_directory, "il2cpp_class_get_type"));
		il2cpp_type_get_object = reinterpret_cast<void* (*)(void*)>(resolveSingleImport(p_export_directory, "il2cpp_type_get_object"));
		il2cpp_class_get_method_from_name = reinterpret_cast<void* (*)(void*, const char*, int)>(resolveSingleImport(p_export_directory, "il2cpp_class_get_method_from_name"));
		il2cpp_class_get_field_from_name = reinterpret_cast<void* (*)(void*, const char*)>(resolveSingleImport(p_export_directory, "il2cpp_class_get_field_from_name"));
		il2cpp_field_get_value = reinterpret_cast<void*(*)(void*, void*, void*)>(resolveSingleImport(p_export_directory, "il2cpp_field_get_value"));

		return true;
	}
};