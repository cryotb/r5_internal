#include <include.h>

UINT32 sc::find_id_by_name(LPCSTR pszName)
{
	constexpr auto nScanSize = 64;

	DWORD_PTR dwAddress = util::find_export<DWORD_PTR>(_XS("ntdll.dll"), pszName);

	if (dwAddress == NULL)
		return 0;

	DWORD_PTR dwDisasmAddr = BASE_OF(dwAddress);
	DWORD dwResult = 0;

	auto sDisasm = hde::disasm_buffer_ex(PTR_OF(dwDisasmAddr), nScanSize);

	for (auto i = 0; i < sDisasm.size(); i++)
	{
		auto sInstr = sDisasm[i];

		if (sInstr.opcode == 0xB8)
		{
			dwResult = *reinterpret_cast<int32_t*>(dwDisasmAddr + 0x1);
			break;
		}

		dwDisasmAddr += sInstr.len;
	}

	return dwResult;
}
