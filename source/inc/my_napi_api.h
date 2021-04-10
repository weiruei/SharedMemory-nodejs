

#ifndef __MY_NAPI_API_H__
#define __MY_NAPI_API_H__



#include <assert.h>
#include <node_api.h>




struct BinaryObj
{
    void *ptr = NULL;
    uint32_t length = 0;
    BinaryObj()
    {
    }

    BinaryObj( const BinaryObj &in)
    {        
        BinaryObj::operator=(in);   
    }
    BinaryObj &operator=(const BinaryObj &in)
    {
        if(ptr != NULL)     delete (unsigned char*)ptr;
        ptr = NULL;     length = 0;
        if( in.ptr == NULL)     return *this;
        length = in.length;
        ptr = new unsigned char[length];     
        memcpy( ptr , in.ptr , in.length);       
        return *this;
    }


    BinaryObj(unsigned long initial_len)
    {
        length = initial_len;   
        ptr = new unsigned char[length];        
    }
    ~BinaryObj()
    {
        if(ptr != NULL)     delete[]( (unsigned char*)ptr);           
    }

    

    unsigned char &operator[](unsigned long index)
    {
        return ((unsigned char*)ptr)[index];
    }

    
    unsigned char *b_str()
    {
        return (unsigned char *)ptr;
    }

    char *c_str()   
    {
        return (char *)ptr;
    }

    // String toString()
    // {
    //     String str;
    //     char t[length + 1];
    //     memcpy( (void*)t , ptr , length );
    //     t[length] = '\0';
    //     str = t;
    //     return str;
    // } 

    BinaryObj &set(void *src , size_t len)
    {
        if(ptr != NULL)     delete (unsigned char*)ptr;
        ptr = NULL;     length = 0;
        if( src == NULL)     return *this; 
        length = len;
        ptr = new unsigned char[length];     
        memcpy( ptr , src , length);   
        return *this;        
    }

    BinaryObj &set(BinaryObj &in)
    {
        return set(in.ptr , in.length);
    }

};




struct ReadArgs
{
    enum {MAX_ARGUMENT_COUNT = 1024};
    size_t argc = MAX_ARGUMENT_COUNT;
    napi_value args[MAX_ARGUMENT_COUNT];
    napi_value jsthis;
    napi_env Env;
    napi_callback_info Info;
    napi_status status;

    napi_value napival;

    ReadArgs(napi_env env, napi_callback_info info)
    {
        Env = env;
        Info = info;
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
    }
    ReadArgs(napi_env env, napi_callback_info info , napi_value _napival)
    {
        Env = env;
        Info = info;
        status = napi_get_cb_info(env, info, &argc, args, &jsthis, nullptr);
        napival = _napival;

        argc = -1;  //-1 mean object is child
    }

    //EXP: function( ["good" , [ "0" , 5] , -1] , 'second arg' );
    //ReadArgs Args(env , info);
    //Args[1].toString();   //GET  'second arg'
    //Args[0][0].toString();   //GET  'good'
    //Args[0][1][0].toString();   //GET  '0'
    //Args[0][1][1].toUint();   //GET  5
    //Args[0][2].toInt64();   //GET  -1
    //Args[0].napival;          //GET ["good" , [ "0" , 5] , -1]
    ReadArgs operator[](int index)
    {
        if((int)argc == -1)
        {
            napi_value child_val;
            napi_get_element(Env, napival, index, &child_val);            
            ReadArgs arg(Env,Info, child_val );
            return arg;   
        }
        ReadArgs arg(Env,Info, args[index] );
        return arg;
    }

    size_t Length()
    {
        return argc;
    }

    napi_valuetype type(int index = -1)
    {   
        // typedef enum {
        //   napi_undefined,
        //   napi_null,
        //   napi_boolean,
        //   napi_number,
        //   napi_string,
        //   napi_symbol,
        //   napi_object,
        //   napi_function,
        //   napi_external,
        //   napi_bigint
        // } napi_valuetype;

        napi_valuetype valuetype;
        status = napi_typeof(Env, (index == -1)?napival:args[index], &valuetype);   
        return status == napi_ok ? valuetype : napi_undefined;
    }

    String toString(int index = -1)
    {
        size_t str_size;
        size_t str_size_read;

        napi_get_value_string_utf8(Env, (index == -1)?napival:args[index], NULL, 0, &str_size);

        
        BinaryObj str(str_size + 1);
        str[str_size] = '\0';

        napi_get_value_string_utf8(Env, (index == -1)?napival:args[index], str.c_str() , str.length, &str_size_read);

        return str.c_str();
    }

    unsigned long toUint(int index = -1)
    {
        uint32_t val;
        status = napi_get_value_uint32( Env , (index == -1)?napival:args[index] , &val);
        return val;
    }

    long long toInt(int index = -1)
    {
        int64_t val;
        status = napi_get_value_int64( Env , (index == -1)?napival:args[index] , &val);
        return val;
    }

    bool toBool(int index = -1)
    {
        bool val;
        status = napi_get_value_bool( Env , (index == -1)?napival:args[index] , &val);
        return val;
    }

    double toDouble(int index = -1)
    {
        double val = 0;
        status = napi_get_value_double( Env , (index == -1)?napival:args[index] , &val);
        return val;
    }

    BinaryObj toBuffer(int index = -1)
    {                 
        size_t len = 0;   
        void *ptr;     
        status = napi_get_buffer_info(Env , (index == -1)?napival:args[index] , &ptr , (size_t *)&len );

        BinaryObj buffer;
        buffer.set(ptr , len);
        
        return buffer;
    }



    napi_ref toFunction(int index = -1)
    {
        napi_ref callbackfun;
        status = napi_create_reference( Env , jsthis , index , &callbackfun); 
        return callbackfun;
    }


    
    // napi_value toObject(int index = -1)
    // {
    //     const char* name;
    //     napi_value ret;
    //     status = napi_get_property( Env ,  (index == -1)?napival:args[index] , name , &ret); 
        
    //     SHOWMESSAGE( name )
    //     return ret;
    // }

    


    void* pThis()
    {
        void* obj;
        status = napi_unwrap(Env, jsthis, reinterpret_cast<void**>(&obj));
        return status == napi_ok ? obj : NULL; 
    }
};






//=========== NAPI RETURN VALUE
napi_value toUndefined(napi_env env)
{
    napi_value ret;
    napi_get_null(env , &ret );
    return ret; 
}
napi_value toInt64(napi_env env, long long v)
{
    napi_value ret;
    napi_create_int64(env , v , &ret );
    return ret; 
}
napi_value toDouble(napi_env env, double v)
{
    napi_value ret;
    napi_create_double(env , v , &ret );
    return ret; 
}
napi_value toUint32(napi_env env, unsigned long v)
{
    napi_value ret;
    napi_create_uint32(env , v , &ret );
    return ret; 
}
napi_value toBool(napi_env env, bool v)
{
    napi_value ret;
    napi_get_boolean(env , v , &ret );
    return ret; 
}
napi_value toString(napi_env env, String v)
{
    napi_value ret;
    napi_create_string_utf8(env , v.c_str() , v.Length() , &ret );
    return ret; 
}
napi_value toBuffer(napi_env env, BinaryObj &v)
{
    // void *ptr = new unsigned char[ v.length ];
    // memcpy( ptr , v.ptr , v.length);    

    // napi_value ret;
    // napi_create_buffer(env , v.length , &ptr , &ret );   //WILL AUTO DELETE ptr   

    // return ret; 


    void *ptr;
    napi_value ret;
    napi_create_buffer_copy(env , v.length , v.ptr , &ptr , &ret );    //AUTO COPY    
    return ret; 
}

//napi_value napi_value_array[5];
//toArray(env , napi_value_array , 5);
napi_value toArray(napi_env env, napi_value *napi_value_array , uint32_t count)  
{
    napi_value array;
    napi_create_array(env, &array);

    for (uint32_t idx = 0; idx < count; idx++) 
    {
        napi_set_element(env, array, idx, napi_value_array[idx]);
    }
    return array; 
}





napi_value toPromise(napi_env env , napi_deferred &deferred )
{
    napi_value promise_value;
    napi_create_promise(env, &deferred, &promise_value );
    return promise_value;
}

napi_value toResolve(napi_env env , napi_deferred &deferred , napi_value ret_value )
{
    napi_resolve_deferred(env, deferred , ret_value);
    return nullptr;
}



#define DECLARE_NAPI_VALUE(name, getfunc,setfun)      {name, 0, 0, getfunc, setfun, 0, napi_default, 0}
#define DECLARE_NAPI_METHOD(name, func)                 { name, 0, func, 0, 0, 0, napi_default, 0 }



#endif