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

#include <cstdint>

#include <string>
#include <vector>

#include <windows.h>

#define AssignImport(import_name) assignImport(import_name, p_export_directory, #import_name);

namespace EzIL2CPP
{
	struct Image
	{
		const char* name;
		const char* name_no_ext;
	};

	struct Assembly
	{
		Image* p_image;
	};

	struct CustomMethodInfo
	{
		struct Argument
		{
			std::string name;
			std::string type;
		};

		void* p_method;
		const char* name;
		bool is_generic;
		uint32_t flags;
		const char* return_type;
		std::vector<Argument> arguments;
	};

	class Resolver
	{
	public:
		void*(*il2cpp_domain_get)();
		Assembly**(*il2cpp_domain_get_assemblies)(void* p_domain, uint64_t* p_assembly_count);
		Assembly*(*il2cpp_domain_assembly_open)(void* p_domain, const char* name);
		Image*(*il2cpp_assembly_get_image)(void* p_assembly);
		void*(*il2cpp_thread_attach)(void* p_domain);
		void*(*il2cpp_thread_detach)(void* p_domain);
		void*(*il2cpp_runtime_invoke)(void* p_method, void* p_object, void** params, void** exceptions);
		void*(*il2cpp_class_from_name)(void* p_image, const char* namespaze, const char* name);
		void*(*il2cpp_class_get_type)(void* p_class);
		void*(*il2cpp_type_get_object)(void* p_type);
		void*(*il2cpp_class_get_method_from_name)(void* p_class, const char* name, int args_count);
		bool(*il2cpp_method_is_generic)(void* p_method);
		void*(*il2cpp_class_get_field_from_name)(void* p_class, const char* name);
		void*(*il2cpp_field_get_value)(void* p_obj, void* p_field, void* p_value);
		void(*il2cpp_field_set_value)(void* obj, void* field, void* value);
		void*(*il2cpp_class_get_methods)(void* p_klass, void* iter);
		const char*(*il2cpp_method_get_name)(void* p_method);
		uint32_t(*il2cpp_method_get_param_count)(void* p_method);
		uint32_t(*il2cpp_method_get_flags)(void* p_method);
		const char*(*il2cpp_method_get_param_name)(void* p_method, uint32_t param_index);
		void*(*il2cpp_method_get_return_type)(void* p_method);
		const char*(*il2cpp_type_get_name)(void* p_type);
		void*(*il2cpp_method_get_param)(void* p_method, uint32_t param_index);
		uint64_t(*il2cpp_image_get_class_count)(Image* p_image);
		void*(*il2cpp_image_get_class)(Image* p_image, uint32_t klass_index);

		Resolver(HMODULE h_game_assembly)
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

		std::vector<const char*> getAssemblyNames(void* p_domain)
		{
			if (!is_initialized)
			{
				setError("getAssemblyNames(): Resolver isn't initialized. Doing this will cause a crash.");
				return {};
			}

			uint64_t assemblies_count;
			std::vector<const char*> assembly_names;
			Assembly** assemblies{ il2cpp_domain_get_assemblies(p_domain, &assemblies_count) };

			for (uint64_t i{}; i < assemblies_count; i++)
				assembly_names.push_back(assemblies[i]->p_image->name);

			return assembly_names;
		}

		void getAssemblyImageClasses(Image* p_image)
		{

		}

		std::vector<CustomMethodInfo> getClassMethods(void* p_klass)
		{
			if (!is_initialized)
			{
				setError("getClassMethods(): Resolver isn't initialized. Doing this will cause a crash.");
				return {};
			}

			std::vector<CustomMethodInfo> methods;

			void* iter{};
			void* p_method{};
			while ((p_method = il2cpp_class_get_methods(p_klass, &iter)) != nullptr)
			{
				const char* method_name{ il2cpp_method_get_name(p_method) };
				bool is_method_generic{ il2cpp_method_is_generic(p_method) };
				uint32_t method_flags{ il2cpp_method_get_flags(p_method) };
				uint32_t params_count{ il2cpp_method_get_param_count(p_method) };
				void* p_return_type{ il2cpp_method_get_return_type(p_method) };
				const char* return_type_name{ il2cpp_type_get_name(p_return_type) };

				methods.push_back({ p_method, method_name, is_method_generic, method_flags, return_type_name ? return_type_name : "void" });
				CustomMethodInfo& custom_method_info{ methods.back() };

				for (uint32_t i{}; i < params_count; i++)
				{
					const char* param_name{ il2cpp_method_get_param_name(p_method, i) };
					void* p_param_type{ il2cpp_method_get_param(p_method, i) };
					const char* param_type_name{ il2cpp_type_get_name(p_param_type) };

					custom_method_info.arguments.push_back({ param_name, param_type_name ? param_type_name : "void" });
				}
			}

			return methods;
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

		void setError(const char* fmt, ...)
		{
			/* Clear error if empty message */
			if (strlen(fmt) == 0)
			{
				has_error = false;
				error_message.clear();
				return;
			}

			va_list args;
			va_start(args, fmt);

			char* buf{ new char[0x1000] };
			vsprintf_s(buf, 0x1000, fmt, args);
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

		template<typename T>
		inline void assignImport(T& p_function, IMAGE_EXPORT_DIRECTORY* p_export_directory, const char* target_export)
		{
			void* result{ resolveSingleImport(p_export_directory, target_export) };
			p_function = reinterpret_cast<decltype(p_function)>(result);
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
			AssignImport(il2cpp_domain_get);
			AssignImport(il2cpp_domain_get_assemblies);
			AssignImport(il2cpp_domain_assembly_open);
			AssignImport(il2cpp_assembly_get_image);
			AssignImport(il2cpp_thread_attach);
			AssignImport(il2cpp_thread_detach);
			AssignImport(il2cpp_runtime_invoke);
			AssignImport(il2cpp_class_from_name);
			AssignImport(il2cpp_class_get_type);
			AssignImport(il2cpp_type_get_object);
			AssignImport(il2cpp_class_get_method_from_name);
			AssignImport(il2cpp_method_is_generic);
			AssignImport(il2cpp_class_get_field_from_name);
			AssignImport(il2cpp_field_get_value);
			AssignImport(il2cpp_field_set_value);
			AssignImport(il2cpp_class_get_methods);
			AssignImport(il2cpp_method_get_name);
			AssignImport(il2cpp_method_get_param_count);
			AssignImport(il2cpp_method_get_flags);
			AssignImport(il2cpp_method_get_param_name);
			AssignImport(il2cpp_method_get_return_type);
			AssignImport(il2cpp_type_get_name);
			AssignImport(il2cpp_method_get_param);
			AssignImport(il2cpp_image_get_class_count);
			AssignImport(il2cpp_image_get_class);

			return true;
		}
	};
}