#include "Cia402device.h"

vector<uint8_t> data32to4x8(uint32_t in);


long CiA402Device::Init(uint8_t new_id)
{
    //id = new_id;
    return 0;
}

CiA402Device::CiA402Device() : CiA301CommPort(1,1) //stdout
{

    comm = 1; //stdout

}

CiA402Device::CiA402Device(uint8_t new_id) : CiA301CommPort(1, new_id)
{

    comm = 1; //stdout
}

CiA402Device::CiA402Device(uint8_t new_id, int fdPort) : CiA301CommPort(fdPort, new_id)
{
    comm = fdPort;

}

long CiA402Device::SwitchOn()
{

//Watch out!! see how to remove sleep commands!!!

//   const vector<u_int8_t> obj2111 ={0x40,0x11,0x21,0x00,0x00};

    long response;
    cerr<<"RESET"<<endl;
    response = WriteNMT(od::reset);

    sleep(1);
    FlushBuffer();
    //OperationMode(od::positionmode);
    cerr<<"START"<<endl;
    WriteNMT(od::start);
    sleep(1);
    FlushBuffer();
    //FlushBuffer();
    WritePDO(od::goreadytoswitchon);
    sleep(1);
    FlushBuffer();

    sleep(1);
    cerr<<"SWITCHON"<<endl;
    WritePDO(od::goswitchon);
    sleep(1);
    FlushBuffer();
    cerr<<"ENABLE"<<endl;
    WritePDO(od::goenable);
    FlushBuffer();

    return 0;
}

long CiA402Device::OperationMode(const vector<uint8_t> mode)
{
    FlushBuffer();

    cout<<"OperationModeDisplay"<<endl;
    ReadSDO(od::OperationModeDisplay);
    FlushBuffer();

    //ask the node for write proper mode in 6060 address
    WriteSDO(od::OperationMode,mode);
    //wait the answer (tx0(580)+id)

    cout<<"OperationModeDisplay"<<endl;
    ReadSDO(od::OperationModeDisplay);
    FlushBuffer();

    return 0;
}


uint16_t CiA402Device::CheckStatus()
{
    //uint16_t* statusp;
    uint16_t status;
    vector<uint8_t> data= od::statusword;
//    data.push_back(od::statusword[0]);
//    data.push_back(od::statusword[1]);

    //Ask for the status word
    status = (uint16_t) ReadSDO(data);
    //status = 0x07;
    //filter state

    return status;

}
void CiA402Device::PrintStatus()
{
    //uint16_t* statusp;
    uint16_t status;
    //vector<uint8_t> data={0x40};
    vector<uint8_t> data=od::statusword;
//    data.push_back(od::statusword[0]);
//    data.push_back(od::statusword[1]);

    //Ask for the status word
    status = (uint16_t) ReadSDO(data);
    cout << "status word: " << std::bitset<16>(status)<< endl;

   // status = 0x07; //testing data
    //filter state
    status = status&0x6f; //mask 01101111=6f

    switch(status)
    {
    case 0x00:
        cout<<"Not Ready to switch on"<<endl;
        break;
    case 0x60:
    case 0x40:
        cout<<"Switch on disabled"<<endl;
        break;
    case 0x21:
        cout<<"Ready to switch on"<<endl;
        break;
    case 0x23:
        cout<<"Switched on"<<endl;
        break;
    case 0x27:
        cout<<"Operation enabled"<<endl;
        break;
    case 0x07:
        cout<<"Quick stop active"<<endl;
        break;
    case 0x0f:
    case 0x2f:
        cout<<"Fault reaction active"<<endl;
        break;
    case 0x08:
    case 0x28:
        cout<<"Fault"<<endl;
        break;
    default:
        cout<<"Not known"<<endl;

    }
    FlushBuffer();
   return;
}

int CiA402Device::CheckError()
{
    //uint16_t* statusp;
    uint32_t error;
    vector<uint8_t> data={0x40};
    data.push_back(od::checkerror[0]);
    data.push_back(od::checkerror[1]);

    //Ask for the status word
    error = (uint32_t) ReadSDO(data);

    cout << "checkerror: " << error << endl;
    cout << "checkerror: " << std::bitset<32>(error)<< endl;
    //Print decoded response for status word
    switch (error)
    {

    case 1: //
        break;

    //default:
    }

    return 0;
}

long CiA402Device::ForceSwitchOff(){
    WritePDO(od::goswitchondisable);
    return 0;
}

long CiA402Device::SwitchOff()
{
    uint16_t status;
    status=CheckStatus();
    //filter state
    status = status&0x6f; //mask 01101111=6f

    switch(status)
    {
//    case 0x00:
//        cout<<"Not Ready to switch on"<<endl;
//        break;
//    case 0x60:
//    case 0x40:
//        cout<<"1"<<endl;
//        break;
    case 0x21:
        //cout<<"2"<<endl;
        cout<<"7"<<endl;
        WritePDO(od::quickstop);
        FlushBuffer();
        break;
    case 0x23:
        cout<<"10"<<endl;
        //WritePDO(od::readytoswitchon);
        //FlushBuffer();
        WritePDO(od::quickstop);
        FlushBuffer();
        break;
    case 0x27:
        cout<<"Leaving Operation enabled"<<endl;
      /*  WritePDO(od::switchon);
        FlushBuffer();
        WritePDO(od::readytoswitchon);
        FlushBuffer();*/
        cout<<"11"<<endl;
        WritePDO(od::quickstop);
        FlushBuffer();
        cout<<"12"<<endl;
        WriteNMT(od::start);
        break;
    case 0x07:
        cout<<"5"<<endl;
        WritePDO(od::quickstop);
        FlushBuffer();
        break;
    case 0x0f:
    case 0x2f:
        cout<<"Fault reaction active"<<endl;
        break;
    case 0x08:
    case 0x28:
        cout<<"Fault"<<endl;
        break;
    default:
        cout<<"Not known"<<endl;

    }
    return 0;
}

long CiA402Device::QuickStop()
{
    const vector<uint8_t> mode = {0x00};
    WriteSDO(od::quick_stop_mode,mode);
    sleep(1);
    WriteSDO(od::stop_option_code,mode);
    sleep(1);
    cout<<"SwitchOff"<<endl;
    WritePDO(od::quickstop);
    FlushBuffer();

    return 0;
}

double CiA402Device::GetPosition()
{

    return (uint32_t) ReadSDO(od::positionaddress);

}

long CiA402Device::SetCommunications(int fdPort)
{
    comm = fdPort;
    return 0;
}

long CiA402Device::SetupPositionMode(/*const vector<uint8_t> target,*/const uint32_t velocity,const uint32_t acceleration /*const vector<uint8_t> deceleration*/){


    OperationMode(od::positionmode);

    // Motion profile type -  trapezoidal
    //WriteSDO(od::motion_profile_type,od::linear_ramp_trapezoidal);
    //Si paso los parametros convertidos en ui, sino convertir primero
    WriteSDO(od::profile_velocity,data32to4x8(velocity));
    //Si paso los parametros convertidos en ui, sino convertir primero
    //WriteSDO(od::profile_acceleration,data32to4x8(acceleration));
//  The target position is the position that the drive should move to in
//  position profile mode using the current settings of motion control parameters
//  such as velocity, acceleration, and motion profile type etc.
//  It is given in position units.
//  Si paso los parametros convertidos en ui, sino convertir primero
    //WriteSDO(od::target_position,target);
    //WriteSDO(od::quick_stop_deceleration,deceleration);
    FlushBuffer();
   return 0;
}

long CiA402Device::Setup_Velocity_Mode(const vector<uint8_t> target,const vector<uint8_t> acceleration){
//    In the Velocity Profile Mode the drive performs speed control.
//    The built-in reference generator computes a speed profile with a trapezoidal shape,
//    due to a limited acceleration. The Target Velocity object specifies
//    the jog speed (speed sign specifies the direction) and the Profile Acceleration
//    object the acceleration/deceleration rate.
    OperationMode(od::velocitymode);
//    The target velocity is the input for the trajectory generator
//    and the value is given in user-defined velocity units.
//    Si paso los parametros convertidos en ui, sino convertir primero
    WriteSDO(od::target_velocity,target);
    WriteSDO(od::profile_acceleration,acceleration);
    return 0;
}

long CiA402Device::SetPosition(uint32_t target){

//WATCH SLEEPS!!!!!!!!!!!!
//remove when working code
    //vector<uint8_t> value;
    //convert target to value

    WriteSDO(od::target_position,data32to4x8(target));
    //WritePDO4(data32to4x8(target));
    sleep(1);
    //setup via control word
//    vector<uint8_t>cw={0x30,0x08,0x00 ,0x00 };
//    WritePDO(cw);
//    sleep(1);
    cerr<<"RUN"<<endl;
      WritePDO(od::run);
      FlushBuffer();
    return 0;
}

vector<uint8_t> data32to4x8(uint32_t in)
{
    vector<uint8_t> retvalue(4);
    retvalue[0] = in&0x000000FF;
    retvalue[1] = (in&0x0000FF00)>>8;
    retvalue[2] = (in&0x00FF0000)>>16;
    retvalue[3] = (in&0xFF000000)>>24;
    cout<< " " <<(int)retvalue[0]
        << " ," <<(int)retvalue[1]
        << " , "<<(int)retvalue[2]
        << " , "<<(int)retvalue[3]<<endl;
    return retvalue;

}
