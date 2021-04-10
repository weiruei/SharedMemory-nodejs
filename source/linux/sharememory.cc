


#include <stdio.h>



#include "../inc/include.h"
#include <fcntl.h>          
#include <sys/stat.h>   
#include <semaphore.h>    
#include <sys/ipc.h>
#include <sys/shm.h>




#include <signal.h>



#include "../inc/my_napi_api.h"


// class MutexOBJ
// {
// public:


//     sem_t *sem;                   /*      synch semaphore         *//*shared */

//     String semName;
    
// MutexOBJ() : sem( NULL )
//     {

//     }
// ~MutexOBJ()
// 	{
//         // SHOWMESSAGE("sem_close");   
//         Close();
// 	}
// void Open( String name )
//     {
//         semName = name;        

//         // sem = sem_open(name.c_str() , O_CREAT | O_EXCL, 0644, 0 ); 

//         sem = sem_open(name.c_str() , O_CREAT , 0644, 1 ); 

//         if( SEM_FAILED == sem)
//         {
//             SHOWMESSAGE("Mutex fail!");   
//         }        
//         else
//         {
//             // int wait_user = 0;
//             // sem_getvalue( sem , &wait_user );
//             // SHOWMESSAGE( (String)"wait_user:" + (String)wait_user );
//         }
//     }

// void Lock( )
// 	{     
//         if( sem != NULL)    
//         {
//             sem_wait( sem );
//         }
// 	}
// void Unlock( )
// 	{
//         if( sem != NULL)    
//         {
//             sem_post( sem );  
//         }  
// 	}

// void Close()
//     {
//         if( sem != NULL)    
//         {            
//             sem_destroy ( sem ); 
//             sem_close ( sem ); 
//             sem_unlink(semName.c_str() ); 
//             // SHOWMESSAGE("SEM_CLOSE");    
//         }
//     }    
// };




class MutexOBJ
{
public:
    sem_t *sem;                   /*      synch semaphore         *//*shared */

    String semName;
    
    bool LockFlag = false;

    struct sigaction sa;
    static void got_signal(int)
    {
        // SHOWMESSAGE("EXIT");
        // SHOWMESSAGE("\r\n");
        exit(EXIT_SUCCESS);
    }
    // void got_signal_fun(int i, siginfo_t *info, void *unknow)
    // {
    //     SHOWMESSAGE( i );
    //     SHOWMESSAGE( (String)(DWORD)info );
    //     SHOWMESSAGE( (String)(DWORD)unknow );
    //     SHOWMESSAGE("EXIT FUN");
    //     exit(EXIT_SUCCESS);
    // }

MutexOBJ() : sem( NULL )
    {        
        memset( &sa, 0, sizeof(sa) );
        sa.sa_handler = MutexOBJ::got_signal;
        // sa.sa_sigaction = got_signal_fun;
        sigfillset(&sa.sa_mask);
        sigaction(SIGINT,&sa,NULL);
    }
~MutexOBJ()
	{
        // SHOWMESSAGE("sem_close");   
        Close();
	}
void Create( String name )
    {
        semName = name;        

        // sem = sem_open(name.c_str() , O_CREAT | O_EXCL, 0644, 0 ); 

        sem = sem_open(name.c_str() , O_CREAT , 0777, 1 ); 

        if( SEM_FAILED == sem)
        {
            SHOWMESSAGE("Mutex fail!");   
        }        
        else
        {
            // SHOWMESSAGE("CREATE NEW MUTEX");
            // int wait_user = 0;
            // sem_getvalue( sem , &wait_user );
            // SHOWMESSAGE( (String)"wait_user:" + (String)wait_user );
        }
    }
void Open( String name )
    {
        semName = name;        

        sem = sem_open(name.c_str() , O_EXCL );

        if( SEM_FAILED == sem)
        {        
            this->Create( name );
        }     

        else
        {
            int index = 255;
            sem_getvalue(sem , &index);            
            // SHOWMESSAGE( (String)"index:" + (String)index );

            for(int i = index ; i > 1; i--) //RELEASE
            {
                sem_wait( sem );  
            }
        }
    }

void Lock( )
	{     
        if( sem != NULL)    
        {            
            sem_wait( sem );
            LockFlag = true;
        }
	}
void Unlock( )
	{
        if( sem != NULL)    
        {            
            sem_post( sem );  
            LockFlag = false;
        }  
	}

void Close()
    {
        if( sem != NULL)    
        {            
            if( LockFlag == true )   
            {
                int index = 255;
                sem_getvalue(sem , &index);
                if( index <= 0)  sem_post( sem );  
            }
            LockFlag = false;

            // sem_destroy ( sem ); 
            // sem_close ( sem ); 
            // sem_unlink(semName.c_str() ); 
            sem = NULL;

            // SHOWMESSAGE("SEM_CLOSE");    
        }
    }    
};


MutexOBJ Mutex;
























class sharememory 
{
protected: 

    String sharememoryName = "";
    key_t ShareMemoryNamekey = -1;
    // MutexOBJ Mutex;
    DWORD PayloadSize = 0,TotalMemorySize = 0;
    int hMapFile = -1;
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

        if( ShareBufferPtr != NULL)     shmdt(ShareBufferPtr);
        
        if(ShareMemoryNamekey >= 0)
        {
            // SHOWMESSAGE("~~~~");
            shmctl(ShareMemoryNamekey, IPC_RMID, NULL) ;   
        }

    }


    WORD crc16c(const char *buf, size_t len , WORD POLY = 0xedb8)
    {
        int k;

        WORD crc = ~0;
        while (len--) {
            crc ^= *buf++;
            for (k = 0; k < 8; k++)
                crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
        }

        return crc;
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

        ShareMemoryNamekey = crc16c( sharememoryName.c_str() , sharememoryName.length() );

        Mutex.Open( sharememoryName );  
        

        if( Args.Length() == 2 )    //CREATE
        {               
            //FIRST TIME CREATE
            TotalMemorySize = Args.toUint(1);  
            DWORD AllocateSize = TotalMemorySize + 4 + 4;


            //CREATE
            hMapFile = shmget( ShareMemoryNamekey , AllocateSize , IPC_CREAT | 0666 );
                    
                    

            if( hMapFile >= 0 )
            {
                
                ShareBufferPtr = (BYTE *)shmat(
                    hMapFile , 
                    NULL , 
                    0   //0:R/W  or SHM_RDONLY 
                );


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
                shmctl(ShareMemoryNamekey, IPC_RMID, NULL) ;
                SHOWMESSAGE( (String)"Create ShareMemoryName ERROR (LastError): " + (String)(int)errno );
                return toBool(env, false);  
            }


        }
        else    //OPEN
        {
            hMapFile = shmget( ShareMemoryNamekey , 0 , 0);//S_IRUSR | 0666 );

            // SHOWMESSAGE( hMapFile );

            if( hMapFile >= 0 )
            {
                ShareBufferPtr = (BYTE *)shmat
                (
                    hMapFile , 
                    NULL , 
                    0   //0:R/W  or SHM_RDONLY 
                );

                TotalMemorySize = ShareBufferPtr[0] << 24;
                TotalMemorySize |= ShareBufferPtr[1] << 16;
                TotalMemorySize |= ShareBufferPtr[2] << 8;
                TotalMemorySize |= ShareBufferPtr[3];      
                
                PayloadSize = ShareBufferPtr[4] << 24;
                PayloadSize |= ShareBufferPtr[5] << 16;
                PayloadSize |= ShareBufferPtr[6] << 8;
                PayloadSize |= ShareBufferPtr[7];    
            }
            else 
            {
                shmctl(ShareMemoryNamekey, IPC_RMID, NULL) ;
                // SHOWMESSAGE( (String)"Open ShareMemoryName ERROR (LastError): " + (String)(int)errno );
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
        if( (hMapFile < 0 ) || (ShareBufferPtr == NULL) )
        {
            return nullptr;   
        }          
        
        
        //GET PAYLOAD SIZE
        PayloadSize = ShareBufferPtr[4] << 24;
        PayloadSize |= ShareBufferPtr[5] << 16;
        PayloadSize |= ShareBufferPtr[6] << 8;
        PayloadSize |= ShareBufferPtr[7];


        BinaryObj BufferTemp(PayloadSize);
        
        if( PayloadSize == 0 )      return toBuffer(env , BufferTemp);
        
        BufferTemp.set(ShareBufferPtr + 8 , PayloadSize);
        // BYTE *ptr = (BYTE*)BufferTemp.b_str();
        // memcpy( ptr , ShareBufferPtr + 8 , PayloadSize);   //MEMORY CPY

           
        
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
        
        if( (hMapFile < 0 ) || (ShareBufferPtr == NULL) )
        {
            return toBool(env, false);    
        }

        ReadArgs Args(env , info);


        if( Args.type(0) != napi_object ) 
        {           
            return toBool(env, false);   
        }


        BinaryObj BufferTemp = Args.toBuffer(0);



        //PUT PAYLOAD SIZE
        PayloadSize = BufferTemp.length;
        ShareBufferPtr[4] = PayloadSize >> 24;
        ShareBufferPtr[5] = PayloadSize >> 16;
        ShareBufferPtr[6] = PayloadSize >> 8;
        ShareBufferPtr[7] = PayloadSize >> 0;
        

        BYTE *ptr = (BYTE*)BufferTemp.b_str();
        memcpy( ShareBufferPtr + 8 , ptr , PayloadSize);   //MEMORY CPY


        return toBool(env, true);
    }
    static napi_value Put(napi_env env, napi_callback_info info)
    {
        ReadArgs Args(env , info);
        sharememory* obj = (sharememory*)(Args.pThis()); 
        napi_value sta = obj->_Put(env , info); 
        return sta;
    }

    static napi_value Lock(napi_env env, napi_callback_info info)
    {
        // ReadArgs Args(env , info);
        // sharememory* obj = (sharememory*)(Args.pThis()); 
        // obj->Mutex.Lock();  
        Mutex.Lock();  
        return toBool(env, true);
    }

    static napi_value Unlock(napi_env env, napi_callback_info info)
    {
        // ReadArgs Args(env , info);
        // sharememory* obj = (sharememory*)(Args.pThis()); 
        // obj->Mutex.Unlock();  
        Mutex.Unlock();  
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
