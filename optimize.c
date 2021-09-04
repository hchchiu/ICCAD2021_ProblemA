/**CFile****************************************************************

  FileName    [demo.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [ABC as a static library.]

  Synopsis    [A demo program illustrating the use of ABC as a static library.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: demo.c,v 1.00 2005/11/14 00:00:00 alanmi Exp $]

***********************************************************************/

#include <stdio.h>
#include <time.h>

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

#if defined(ABC_NAMESPACE)
namespace ABC_NAMESPACE
{
#elif defined(__cplusplus)
extern "C"
{
#endif

// procedures to start and stop the ABC framework
// (should be called before and after the ABC procedures are called)
void   Abc_Start();
void   Abc_Stop();

// procedures to get the ABC framework and execute commands in it
typedef struct Abc_Frame_t_ Abc_Frame_t;

Abc_Frame_t * Abc_FrameGetGlobalFrame();
int    Cmd_CommandExecute( Abc_Frame_t * pAbc, const char * sCommand );

#if defined(ABC_NAMESPACE)
}
using namespace ABC_NAMESPACE;
#elif defined(__cplusplus)
}
#endif

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [The main() procedure.]

  Description [This procedure compiles into a stand-alone program for 
  DAG-aware rewriting of the AIGs. A BLIF or PLA file to be considered
  for rewriting should be given as a command-line argument. Implementation 
  of the rewriting is inspired by the paper: Per Bjesse, Arne Boralv, 
  "DAG-aware circuit compression for formal verification", Proc. ICCAD 2004.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int main( int argc, char * argv[] )
{
    // parameters
    int fUseResyn2  = 1;
    int fPrintStats = 1;
    int fVerify     = 1;
    // variables
    Abc_Frame_t * pAbc;
    char * pFileName;
    char Command[1000];
    clock_t clkRead, clkResyn, clkVer, clk;

    //////////////////////////////////////////////////////////////////////////
    // get the input file name
    if ( argc != 2 )
    {
        printf( "Wrong number of command-line arguments.\n" );
        return 1;
    }
    pFileName = argv[1];

    //////////////////////////////////////////////////////////////////////////
    // start the ABC framework
    Abc_Start();
    pAbc = Abc_FrameGetGlobalFrame();

clk = clock();
    //////////////////////////////////////////////////////////////////////////
    // read the file
    sprintf( Command, "read %s", pFileName );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;
    }
    /*sprintf( Command, "strash");
    if(Cmd_CommandExecute( pAbc, Command )){
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;
    }*/
    //////////////////////////////////////////////////////////////////////////
    // balance
    sprintf( Command, "balance" );
    fprintf(stdout, "Execute balance ...\n");
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;
    }

    //////////////////////////////////////////////////////////////////////////
    // print stats
    if ( fPrintStats )
    {
        sprintf( Command, "print_stats" );
        if ( Cmd_CommandExecute( pAbc, Command ) )
        {
            fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
            return 1;
        }
    }
    
    
    //////////////////////////////////////////////////////////////////////////
    // synthesize
    /**/for (int i = 0; i < 10; ++i) {
        fprintf(stdout, "Execute Resyn2 ...\n");
        if (fUseResyn2)
        {
            sprintf(Command, "balance; rewrite -l; refactor -l; balance; rewrite -l; rewrite -lz; balance; refactor -lz; rewrite -lz; balance");
            if (Cmd_CommandExecute(pAbc, Command))
            {
                fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
                return 1;
            }
        }
        else
        {
            sprintf(Command, "balance; rewrite -l; rewrite -lz; balance; rewrite -lz; balance");
            if (Cmd_CommandExecute(pAbc, Command))
            {
                fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
                return 1;
            }
        }
    }
clkResyn = clock() - clk;

    //////////////////////////////////////////////////////////////////////////
    // print stats
    if ( fPrintStats )
    {
        sprintf( Command, "print_stats" );
        if ( Cmd_CommandExecute( pAbc, Command ) )
        {
            fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
            return 1;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    //dc2
    for (int i = 0; i < 3; ++i) {
        fprintf(stdout, "Execute dc2 ...\n");
        sprintf(Command, "dc2");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
        //////////////////////////////////////////////////////////////////////////
        // print stats
        if (fPrintStats)
        {
            sprintf(Command, "print_stats");
            if (Cmd_CommandExecute(pAbc, Command))
            {
                fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
                return 1;
            }
        }
    }
    //////////////////////////////////////////////////////////////////////////
    //fraig
    fprintf(stdout, "Execute fraig ...\n");
    sprintf(Command, "fraig");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    // print stats
    if (fPrintStats)
    {
        sprintf(Command, "print_stats");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    // balance
    sprintf(Command, "balance");
    fprintf(stdout, "Execute balance ...\n");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }

    //////////////////////////////////////////////////////////////////////////
    // print stats
    if (fPrintStats)
    {
        sprintf(Command, "print_stats");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    // abc9 get
    /*fprintf(stdout, "Execute &get ...\n");
    sprintf(Command, "&get");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    //sweep
    fprintf(stdout, "Execute &sweep ...\n");
    sprintf(Command, "&sweep");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    // print stats
    if (fPrintStats)
    {
        sprintf(Command, "&ps");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
    }*/
    //////////////////////////////////////////////////////////////////////////
    //dc2
    /*for (int i = 0; i < 3; ++i) {
        fprintf(stdout, "Execute &dc2 ...\n");
        sprintf(Command, "&dc2");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
        //////////////////////////////////////////////////////////////////////////
        // print stats
        if (fPrintStats)
        {
            sprintf(Command, "&ps");
            if (Cmd_CommandExecute(pAbc, Command))
            {
                fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
                return 1;
            }
        }
    }*/
    //////////////////////////////////////////////////////////////////////////
    // syn4
    /*fprintf(stdout, "Execute &syn4 ...\n");
    sprintf(Command, "&syn4");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    clkRead = clock() - clk;*/

    //////////////////////////////////////////////////////////////////////////
    // print stats
    /*if (fPrintStats)
    {
        sprintf(Command, "print_stats");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    //sweep
    fprintf(stdout, "Execute &sweep ...\n");
    sprintf(Command, "&sweep");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    // print stats
    if (fPrintStats)
    {
        sprintf(Command, "&ps");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
    }*/
    //////////////////////////////////////////////////////////////////////////
    //cfraig
    /*fprintf(stdout, "Execute &cfraig ...\n");
    sprintf(Command, "&cfraig");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    // print stats
    if (fPrintStats)
    {
        sprintf(Command, "&ps");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
    }*/
    //////////////////////////////////////////////////////////////////////////
    //fraig
    /*fprintf(stdout, "Execute &fraig ...\n");
    sprintf(Command, "&fraig");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    // print stats
    if (fPrintStats)
    {
        sprintf(Command, "&ps");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
    }
    //////////////////////////////////////////////////////////////////////////
    //dc2
    for (int i = 0; i < 2; ++i) {
        fprintf(stdout, "Execute &dc2 ...\n");
        sprintf(Command, "&dc2");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
        //////////////////////////////////////////////////////////////////////////
        // print stats
        if (fPrintStats)
        {
            sprintf(Command, "&ps");
            if (Cmd_CommandExecute(pAbc, Command))
            {
                fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
                return 1;
            }
        }
    }*/
    //////////////////////////////////////////////////////////////////////////
    // abc9 put
    /*fprintf(stdout, "Execute &put ...\n");
    sprintf(Command, "&put");
    if (Cmd_CommandExecute(pAbc, Command))
    {
        fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    // print stats
    if (fPrintStats)
    {
        sprintf(Command, "print_stats");
        if (Cmd_CommandExecute(pAbc, Command))
        {
            fprintf(stdout, "Cannot execute command \"%s\".\n", Command);
            return 1;
        }
    }*/
    //////////////////////////////////////////////////////////////////////////
    // write the result in blif
    sprintf( Command, "write_blif opt_patch.blif" );
    if ( Cmd_CommandExecute( pAbc, Command ) )
    {
        fprintf( stdout, "Cannot execute command \"%s\".\n", Command );
        return 1;
    }
    //////////////////////////////////////////////////////////////////////////
    // perform verification
clk = clock();

    //printf( "Reading = %6.2f sec   ",     (float)(clkRead)/(float)(CLOCKS_PER_SEC) );
    //printf( "Rewriting = %6.2f sec   ",   (float)(clkResyn)/(float)(CLOCKS_PER_SEC) );
    //printf( "Verification = %6.2f sec\n", (float)(clkVer)/(float)(CLOCKS_PER_SEC) );

    //////////////////////////////////////////////////////////////////////////
    // stop the ABC framework
    Abc_Stop();
    return 0;
}

