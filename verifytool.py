from optparse import OptionParser, OptionGroup
from components.log import Log
from os.path import exists
from filecmp import cmp
from shutil import copyfile

def must_exist( path ):
    if not exists( path ):
        Log.error( f"  [✘] {path} - Missing require file!" )
        return False
    Log.info( f"  [✔] {path}" )
    return True

def must_be_identical( path, orig, fix = False ):
    if not cmp( path, orig ):
        Log.error( f"  [✘] {path} - File differs from upstream." )
        if fix:
            Log.warn( "        - Attempting to copy from known-good source..." )
            copyfile( orig, path )
            return must_be_identical( path, orig, fix = False );
        return False
    Log.info( f"  [✔] {path}" )
    return True

def run_tool( options, args ):

    Log.info( "Checking the project files..." )
    buildOK = True
    buildOK &= must_be_identical( 'codal.py', 'libraries/codal/tool.py', fix = options.fix )
    buildOK &= must_exist( 'codal.json' )
    buildOK &= must_be_identical( 'CMakeLists.txt', 'libraries/codal/CMakelists.txt', fix = options.fix )
    buildOK &= must_exist( 'source' )
    buildOK &= must_exist( 'source/main.cpp' )
    buildOK &= must_exist( 'libraries' )

    if not buildOK:
        Log.error( "!!! One or more checks failed, your build my not work !!!" )
        Log.warn( "You might want to try to run 'verify --fix' to attempt to automatically fix this. (BETA)" )
    else:
        Log.info( "Verify - OK" )


def get_opt_parser():
    parser = OptionParser(
        usage="VERIFY: codal.py verify",
        description="Attempts to verify if the build configuration is valid"
    )
    parser.add_option( '--fix', dest='fix', action='store_true', help='Attempt to fix the build, if possible. (BETA Feature)', default=False )
    return parser

if __name__ == "__main__":
    parser = get_opt_parser()
    (options, args) = parser.parse_args()
    run_tool( options, args )
    exit( 0 )