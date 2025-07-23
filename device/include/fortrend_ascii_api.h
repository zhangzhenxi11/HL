// Library: LibDevicePlugin
// Package: Api/ASCII
//
// piotech api 
//
// author xielonghua
//


#ifndef _XLH_FORTREND_EFEM_ASCII_API_INCLUDE_
#define _XLH_FORTREND_EFEM_ASCII_API_INCLUDE_
#include  "efem_ascii_api.h"
#include  "Kernel/kernel_event_module.h"
#include  "Kernel/kernel_abstract_iocontrol.h"
#include  <memory>
#include  <functional>
#include  <map>

#include "EFEM/efem_loadport_subsystem.h"
#include "EFEM/efem_wafer_robot_subsystem.h"

namespace FC{
class IKernel;
class IKernelSubSystem;
class Command;

static Cassette::Mapping getMappingFromChar(char str){
	Cassette::Mapping ret = Cassette::Mapping::Unknown;
	switch (str)
	{
	case 'P':
		ret = Cassette::Mapping::Present;
		break;
	case 'E':
		ret = Cassette::Mapping::Empty;
		break;
	case 'C':
		ret = Cassette::Mapping::Crossed;
		break;
	case 'D':
		ret = Cassette::Mapping::Double;
		break;
	default:
		break;
	}
	return ret;
}

/**
* pio tech api
*/
class  FortrendAsciiEFEMApi : public EFEMAsciiApi, public  KernelEventListener, public KernelListener<KernelAbstractIOControl>
{
public:
	FortrendAsciiEFEMApi(IKernel*  kernel);

public:
	typedef enum{ IDEL, WAIT_REPLY, REQUEST_FAILD, RESPONSE_TIMEOUT, FINISHED }  CmdState;


private:
	void handle_INIT(const std::shared_ptr<Command>& command);	//INIT
	void handle_LOCK(const std::shared_ptr<Command>& command);	//LOCK
	void handle_UNLOCK(const std::shared_ptr<Command>& command);//UNLOCK
	void handle_OPEN(const std::shared_ptr<Command>& command);	//OPEN
	void handle_CLOSE(const std::shared_ptr<Command>& command);	//CLOSE
	void handle_WAFSH(const std::shared_ptr<Command>& command);	//WAFSH
	void handle_MAPDT(const std::shared_ptr<Command>& command);//MAPDT
	void handle_GOTO(const std::shared_ptr<Command>& command);//GOTO
	void handle_LOAD(const std::shared_ptr<Command>& command);//LOAD
	void handle_UNLOAD(const std::shared_ptr<Command>& command);//UNLOAD
	void handle_ALIGN(const std::shared_ptr<Command>& command);//ALIGN
	void handle_MODE(const std::shared_ptr<Command>& command);//MODE
	void handle_SIGOUT(const std::shared_ptr<Command>& command);//SIGOUT
	void handle_STATE(const std::shared_ptr<Command>& command);//STATE
	void handle_SIGSTAT(const std::shared_ptr<Command>& command);//SIGSTAT
	void handle_TOWER(const std::shared_ptr<Command>& command);//TOWER
	void handle_CSTID(const std::shared_ptr<Command>& command);//CSTID

	void handle_SPEED(const std::shared_ptr<Command>& command);//SPEED
	void handle_CONTROLMODE(const std::shared_ptr<Command>& command);//CONTROLMODE
	void handle_HEARTBEAT(const std::shared_ptr<Command>& command);//HEARTBEAT
	void handle_TOOLSLOT(const std::shared_ptr<Command>& command);//TOOLSLOT
	void handle_CLAMP(const std::shared_ptr<Command>& command);//CLAMP
	void handle_TRIGGER(const std::shared_ptr<Command>& command);//TRIGGER
	void handle_ALARM(const std::shared_ptr<Command>& command);//ALARM
	void handle_POD(const std::shared_ptr<Command>& command);//POD
	void handle_DOORSTAT(const std::shared_ptr<Command>& command);//DOORSTAT
	
	void handle_HOLD(const std::shared_ptr<Command>& command);//HOLD
	void handle_RESTR(const std::shared_ptr<Command>& command);//RESTR
	void handle_ABORT(const std::shared_ptr<Command>& command);//ABORT

	void handle_TRANSF(const std::shared_ptr<Command>& command);//TRANSF

protected:
	virtual void initialize() throw(KernelException)override;
	virtual void unInitialize()override;
	virtual bool hookEnable()override;
	virtual void handle(const std::shared_ptr<Command>& command)override;
	virtual void update()override;
private:
	void handle_load_unload(const std::shared_ptr<Command>& command,bool load);

	void onHandleEvent(KernelEventModule* kernelModule, const std::shared_ptr<IEventId>& evtId, KernelEventParameter* context);	
	void onHandleAlarm(KernelEventModule* kernelModule, const std::shared_ptr<IEventId>& evtId, KernelEventParameter* context);

	static std::shared_ptr<Message> createMessageFrom(const IKernelIOControl& io_control);
	void onAttributeChange(const KernelAbstractIOControl *arg) override;
	void setCmdState(CmdState state);

public:
	CmdState getCmdState()const;
	void wait();
	bool hasFinishedState()const;
	void  FortrendAsciiEFEMApi::submitTask(Base base, const::std::string Name,const::std::string& command);
public:
	//EFEMLPSubsystem* pLoadport;
	//EFEMWaferRobotSubsystem* pwtr;
private:
	virtual void onKernelEvent(KernelEventModule* kernelModule, const std::shared_ptr<IEventId>& eventId, KernelEventParameter* context)  override;
private:
	DECLARE_PRIVATE(FortrendAsciiEFEMApi);
	std::map<Base, std::function<void(const std::shared_ptr<Command>&)>> handleMap;
	std::map<Base, std::map<std::string, CmdState>> stateMap;
};

}
#endif
