

//sharememory ALL METHOD LIST

//CREATE NEW MEMORY
//RETURN true OR false
//sharememory.Open( unique_name , size_in_byte);      

//OPEN EXIST MEMORY
//RETURN true OR false
//sharememory.Open( unique_name);      

//PUT DATA TO MEMORY
//sharememory.Put( array_buffer );     

//GET DATA FROM MEMORY
//ALWAYS RETURN array_buffer
//array_buffer = sharememory.Get();  

//LOCK SHARED MEMORY BEFORE MODIFY THE CONTENT (MUTEX)
//IT WILL BLOCK IN HERE IF OTHER PROCESS LOCKED ALREADY (NOT PROMISE TYPE)
//sharememory.Lock();


//UNLOCK SHARED MEMORY
//sharememory.Unlock();




const sharememory = new (require( './sharememory.node')).sharememory(); 



(async function(){

    
    while(true)
    {
        let sta = sharememory.Open("MyMemory");
        console.log( sta );
        if( sta === true)  break;  //WAIT FOR OPEN

        await Delayms(200);
    }
    

    sharememory.Lock();

    var buf = sharememory.Get();

    sharememory.Unlock();


    console.log( buf.toString() );

    
    //Infinity loop
    while(true)
    {
        await Delayms(200);    
    }

})();






function Delayms(ms)
{    
    let Resolve;

    if( ms >= 0)
        setTimeout(function(){Resolve();} , ms);
    else    //Infinity
        ;        

    return new Promise(function(resolve,reject){
        Resolve = resolve;
    });
}