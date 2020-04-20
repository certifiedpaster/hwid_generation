#include <iostream>
#include <string>
#include <windows.h>

#include "picosha2.h"
#include "smbios.hpp"

using namespace smbios;

int main()
{
	// Query size of SMBIOS data.
	const DWORD smbios_data_size = GetSystemFirmwareTable('RSMB', 0, nullptr, 0);

	// Allocate memory for SMBIOS data
	const auto heap_handle = GetProcessHeap();
	const auto smbios_data = static_cast<raw_smbios_data*>(HeapAlloc(heap_handle, 0,
	                                                                 static_cast<size_t>(smbios_data_size)));
	if (!smbios_data)
	{
		return 0;
	}

	// Retrieve the SMBIOS table
	const DWORD bytes_written = GetSystemFirmwareTable('RSMB', 0, smbios_data, smbios_data_size);
	if (bytes_written != smbios_data_size)
	{
		return 0;
	}

	// Process the SMBIOS data and free the memory under an exit label
	parser meta;
	const auto buff = smbios_data->smbios_table_data;
	const auto buff_size = static_cast<size_t>(smbios_data_size);

	meta.feed(buff, buff_size);

	std::string hardware;

	for (auto& header : meta.headers)
	{
		string_array_t strings;
		parser::extract_strings(header, strings);

		switch (header->type)
		{
		case types::baseboard_info:
			{
				const auto x = reinterpret_cast<baseboard_info*>(header);

				if (x->length == 0)
					break;

				hardware.append(strings[x->manufacturer_name]);
				hardware.append(strings[x->product_name]);
			}
			break;

		case types::bios_info:
			{
				const auto x = reinterpret_cast<bios_info*>(header);

				if (x->length == 0)
					break;
				hardware.append(strings[x->vendor]);
				hardware.append(strings[x->version]);
			}
			break;


		case types::memory_device:
			{
				const auto x = reinterpret_cast<mem_device*>(header);

				if (x->total_width == 0)
					break;
				hardware.append(strings[x->manufacturer]);
				hardware.append(strings[x->serial_number]);
				hardware.append(strings[x->part_number]);
			}
			break;
		case types::processor_info:
			{
				const auto x = reinterpret_cast<proc_info*>(header);

				if (x->length == 0)
					break;
				hardware.append(strings[x->manufacturer]);
				hardware.append(strings[x->version]);
				hardware.append(std::to_string(static_cast<long>(x->id)));
			}
			break;

		default: ;
		}
	}

	HeapFree(heap_handle, 0, smbios_data);

	std::string hash_hex_str;
	picosha2::hash256_hex_string(hardware, hash_hex_str);
	std::cout << "string: " << hardware << std::endl;
	std::cout << "hwid sha256: " << hash_hex_str << std::endl;

	getchar();
	return 0;
}
