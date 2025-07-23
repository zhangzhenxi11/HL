/**
 * @file            rnd_aligner_ocr_command.h
 * @brief           ocr command for rnd  aligner subsystem
 * @author			xielonghua
 */

 // Library: Fortrend
 // Package: CommandImp/Rnd/Aligner

#ifndef _XLH_EFEM_OCR_COMMAND_INCLUDE_
#define _XLH_EFEM_OCR_COMMAND_INCLUDE_

#include "Kernel/Fortrend/hex_command_executer.h"
#include "Kernel/Fortrend/aligner_abstract_command.h"
#include "Kernel/kernel_exception.h" 

class HexSubSystemHelper;
namespace FC {

	/**
	*@brief   Ocr command for rnd  aligner subsystem
	*/
	class  EFEMAlignerOcrCommand : public KernelSubsystemCommand, public HexCommandExecuter
	{
	public:
		DECLARE_PTR(EFEMAlignerOcrCommand)
		EFEMAlignerOcrCommand(HexSubSystemHelper* hexHelper, int dirct); //1:正向 2：反向

		virtual std::string getName() const override { return "OCR"; }

	protected:
		virtual RunResult onRun() throw(KernelException)override;
	private:
		DECLARE_PRIVATE(EFEMAlignerOcrCommand)
	};


}

#endif
