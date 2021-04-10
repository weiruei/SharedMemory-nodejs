



#include <Windows.h>


#include "../inc/include.h"


#include "../inc/my_napi_api.h"




//===========================================
class MutexOBJ
{
public:
	HANDLE MUTEX = NULL;    
    
MutexOBJ()  {}
~MutexOBJ()
	{
        // UnLock();
        CloseHandle(MUTEX);
	}
void Open( String name )
    {
        MUTEX = CreateMutex(NULL, FALSE , name.c_str() );
    }

void Lock( )
	{     

		QWORD status;
		do
		{
			status = WaitForSingleObject(MUTEX,INFINITE);
//			if( status == WAIT_ABANDONED)
			if(
				( status == WAIT_ABANDONED) ||
				( status == WAIT_TIMEOUT) ||
				( status == WAIT_FAILED)
			)
			{                
				ReleaseMutex(MUTEX);
                SHOWMESSAGE( "MUTEX ABORT" );
				break;
			}
			// if( status != WAIT_OBJECT_0)	Sleep(0);
		}
		while( status != WAIT_OBJECT_0);
	}
void Unlock( )
	{
		ReleaseMutex(MUTEX);        
	}

void Close()
    {
        CloseHandle(MUTEX);
    }    


};



















class sharememory 
{
protected: 

    String sharememoryName = "";
    MutexOBJ Mutex;
    DWORD PayloadSize = 0,TotalMemorySize = 0;
    HANDLE hMapFile = NULL;
    BYTE *ShareBufferPtr = NULL;

    static napi_ref constructor;
    
    napi_env env_;
    napi_ref wrapper_;

public:
    static napi_value Init(napi_env env, napi_value exports)
    {
        napi_status status;
        napi_property_descriptor properties[] = 
        {            
            DECLARE_NAPI_METHOD("Open", Open),
            DECLARE_NAPI_METHOD("Put", Put),
            DECLARE_NAPI_METHOD("Get", Get),
            DECLARE_NAPI_METHOD("Lock", Lock),
            DECLARE_NAPI_METHOD("Unlock", Unlock),

            DECLARE_NAPI_VALUE("length" , Glength, Slength),
            DECLARE_NAPI_VALUE("size" , Gsize, Ssize),
        };

        napi_value cons;
        status = napi_define_class(
            env, "sharememory", NAPI_AUTO_LENGTH, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor) , properties, &cons);
        assert(status == napi_ok);


        status = napi_create_reference(env, cons, 1, &constructor);
        assert(status == napi_ok);

        status = napi_set_named_property(env, exports, "sharememory", cons);
        assert(status == napi_ok);
        return exports;
    }
    static void Destructor(napi_env env, void* nativeObject, void* finalize_hint)
    {
        reinterpret_cast<sharememory*>(nativeObject)->~sharememory();
    }

private:
    explicit sharememory() : env_(nullptr), wrapper_(nullptr) 
    {
    }

    ~sharememory()
    {
        napi_delete_reference(env_, wrapper_);
    }


    


    static napi_value New(napi_env env, napi_callback_info info)
    {
        napi_status status;

        napi_value target;
        status = napi_get_new_target(env, info, &target);
        assert(status == napi_ok);
        bool is_constructor = target != nullptr;

        if (is_constructor) 
        {
            // Invoked as constructor: `new sharememory(...)`
            ReadArgs Args(env , info);

            sharememory* obj = new sharememory();

            obj->env_ = Args.Env;
            status = napi_wrap(Args.Env,
                Args.jsthis,
                reinterpret_cast<void*>(obj),
                sharememory::Destructor,
                nullptr,  // finalize_hint
                &obj->wrapper_);
            assert(status == napi_ok);

            return Args.jsthis;
        } 
        else 
        {
            // Invoked as plain function `sharememory(...)`, turn into construct call.           

            napi_value cons;
            status = napi_get_reference_value(env, constructor, &cons);
            assert(status == napi_ok);

            napi_value instance;
            size_t argc = 0;
            status = napi_new_instance(env, cons, argc, nullptr, &instance);
            assert(status == napi_ok);

            return instance;
        }
    }


    napi_value _Open(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);

        sharememoryName = Args.toString(0);        


        Mutex.Open( sharememoryName );  
        

        if( Args.Length() == 2 )    //CREATE
        {               

            //FIRST TIME CREATE
            TotalMemorySize = Args.toInt(1);  
            DWORD AllocateSize = TotalMemorySize + 4 + 4;


            //CREATE
            hMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,    // use paging file
                NULL,                    // default security
                PAGE_READWRITE,          // read/write access
                0,                       // maximum object size (high-order DWORD)
                AllocateSize,                // maximum object size (low-order DWORD)
                ("Global\\" + sharememoryName).c_str());                    // name of mapping object

            // SHOWMESSAGE( GetLastError() );

            // SHOWMESSAGE( (String)"TotalSize:" + (String)TotalSize );

            // SHOWMESSAGE( "Global\\" + sharememoryName );

            if( hMapFile != NULL )
            {
                ShareBufferPtr = (BYTE *)MapViewOfFile(hMapFile,   // handle to map object
                    FILE_MAP_ALL_ACCESS, // read/write permission
                    0,
                    0,
                    AllocateSize);

                ShareBufferPtr[0] = TotalMemorySize >> 24;
                ShareBufferPtr[1] = TotalMemorySize >> 16;
                ShareBufferPtr[2] = TotalMemorySize >> 8;
                ShareBufferPtr[3] = TotalMemorySize >> 0;
                
                
                //PUT PAYLOAD SIZE
                ShareBufferPtr[4] = PayloadSize >> 24;
                ShareBufferPtr[5] = PayloadSize >> 16;
                ShareBufferPtr[6] = PayloadSize >> 8;
                ShareBufferPtr[7] = PayloadSize >> 0;
            }
            else 
            {
                SHOWMESSAGE( (String)"Open sharememoryName ERROR (LastError): " + (String)GetLastError() );

                return toBool(env, false);    
            }
        }
        else    //OPEN
        {
            hMapFile = OpenFileMapping(
                            FILE_MAP_ALL_ACCESS,   // read/write access
                        FALSE,                 // do not inherit the name
                        ("Global\\" + sharememoryName).c_str());               // name of mapping object

            if( hMapFile != NULL )
            {
                //GET SIZE FIRST
                ShareBufferPtr = (BYTE *)MapViewOfFile(hMapFile,   // handle to map object
                    FILE_MAP_ALL_ACCESS, // read/write permission
                    0,
                    0,
                    8);


                TotalMemorySize = ShareBufferPtr[0] << 24;
                TotalMemorySize |= ShareBufferPtr[1] << 16;
                TotalMemorySize |= ShareBufferPtr[2] << 8;
                TotalMemorySize |= ShareBufferPtr[3];         

                PayloadSize = ShareBufferPtr[4] << 24;
                PayloadSize |= ShareBufferPtr[5] << 16;
                PayloadSize |= ShareBufferPtr[6] << 8;
                PayloadSize |= ShareBufferPtr[7];               


                //REMAP
                ShareBufferPtr = (BYTE *)MapViewOfFile(hMapFile,   // handle to map object
                    FILE_MAP_ALL_ACCESS, // read/write permission
                    0,
                    0,
                    TotalMemorySize + 8);
            }
            else 
            {
                return toBool(env, false);     
            }
        }
        

        
        return toBool(env, true);  
    }
    static napi_value Open(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);
        sharememory* obj = (sharememory*)(Args.pThis()); 
        return obj->_Open(env , info);          
    }


    napi_value _Get(napi_env env, napi_callback_info info)
    {
        if( (hMapFile == NULL) || (ShareBufferPtr == NULL) )
        {
            return nullptr;   
        }                 

        //GET PAYLOAD SIZE
        PayloadSize = ShareBufferPtr[4] << 24;
        PayloadSize |= ShareBufferPtr[5] << 16;
        PayloadSize |= ShareBufferPtr[6] << 8;
        PayloadSize |= ShareBufferPtr[7];

        BYTE* memptr = ShareBufferPtr;
        memptr += 8;

        BinaryObj BufferTemp(PayloadSize);
        
        if( PayloadSize == 0 )      return toBuffer(env , BufferTemp);
        // SHOWMESSAGE( PayloadSize );

        BYTE *ptr = (BYTE*)BufferTemp.b_str();

        DWORD l = PayloadSize >> 2;
        for( DWORD i = 0 ; i < l ; i++ )  //MEMORY CPY
        {        
            ((DWORD*)ptr)[i] = ((DWORD*)(memptr))[i];
        }
        
        for( DWORD i = PayloadSize - (PayloadSize % 4) ; i < PayloadSize; i++)     //MEMORY CPY
        {
            ptr[i] = memptr[i];
        }

        return toBuffer(env , BufferTemp);             
    }
    static napi_value Get(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);
        sharememory* obj = (sharememory*)(Args.pThis()); 
        return obj->_Get(env , info); 
    }



    napi_value _Put(napi_env env, napi_callback_info info)
    {
        if( (hMapFile == NULL) || (ShareBufferPtr == NULL) )
        {
            return toBool(env, false);    
        }     

        ReadArgs Args(env , info);

        BinaryObj BufferTemp = Args.toBuffer(0);


        //PUT PAYLOAD SIZE
        PayloadSize = BufferTemp.length;
        ShareBufferPtr[4] = PayloadSize >> 24;
        ShareBufferPtr[5] = PayloadSize >> 16;
        ShareBufferPtr[6] = PayloadSize >> 8;
        ShareBufferPtr[7] = PayloadSize >> 0;

        BYTE* memptr = ShareBufferPtr;
        memptr += 8;

        BYTE *ptr = (BYTE*)BufferTemp.b_str();

        DWORD l = PayloadSize >> 2;
        for( DWORD i = 0 ; i < l ; i++ )  //MEMORY CPY
        {        
            ((DWORD*)(memptr))[i] = ((DWORD*)ptr)[i];
            // SHOWMESSAGE( (String)ShareBuffer[i] );
        }
        
        for( DWORD i = PayloadSize - (PayloadSize % 4) ; i < PayloadSize; i++)     //MEMORY CPY
        {
            memptr[i] = ptr[i];
        }


        return toBool(env, true);
    }
    static napi_value Put(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);
        sharememory* obj = (sharememory*)(Args.pThis()); 
        return obj->_Put(env , info); 
    }

    static napi_value Lock(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);
        sharememory* obj = (sharememory*)(Args.pThis()); 
        obj->Mutex.Lock();  
        return toBool(env, true);
    }

    static napi_value Unlock(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);
        sharememory* obj = (sharememory*)(Args.pThis()); 
        obj->Mutex.Unlock();  
        return toBool(env, true);
    }




    static napi_value Glength(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);
        sharememory* obj = (sharememory*)(Args.pThis()); 
        return toInt64(env, obj->PayloadSize);
    }
    static napi_value Slength(napi_env env, napi_callback_info info)
    {   
        return nullptr;
    }
    static napi_value Gsize(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);
        sharememory* obj = (sharememory*)(Args.pThis()); 
        return toInt64(env, obj->TotalMemorySize);
    }
    static napi_value Ssize(napi_env env, napi_callback_info info)
    {    
        return nullptr;
    }

    








};




napi_ref sharememory::constructor;









napi_value Init(napi_env env, napi_value exports) 
{
  return sharememory::Init(env, exports);
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
