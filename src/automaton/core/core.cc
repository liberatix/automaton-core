#include <iostream>
#include <string>

#include "automaton/core/cli/cli.h"

/*

██     ▄     ▄▄▄▄▀ ████▄ █▀▄▀█ ██     ▄▄▄▄▀ ████▄    ▄   
█ █     █ ▀▀▀ █    █   █ █ █ █ █ █ ▀▀▀ █    █   █     █  
█▄▄█ █   █    █    █   █ █ ▄ █ █▄▄█    █    █   █ ██   █ 
█  █ █   █   █     ▀████ █   █ █  █   █     ▀████ █ █  █ 
   █ █▄ ▄█  ▀               █     █  ▀            █  █ █ 
  █   ▀▀▀                  ▀     █                █   ██ 
 ▀                              ▀                        
▄█▄    ████▄ █▄▄▄▄ ▄███▄                   ▄             
█▀ ▀▄  █   █ █  ▄▀ █▀   ▀                   █            
█   ▀  █   █ █▀▀▌  ██▄▄                █     █           
█▄  ▄▀ ▀████ █  █  █▄   ▄▀              █    █           
▀███▀          █   ▀███▀                 █  █ ██ ██      
              ▀                           █▐             
                                          ▐              


     _   _   _ _____ ___  __  __    _  _____ ___  _   _ 
    / \ | | | |_   _/ _ \|  \/  |  / \|_   _/ _ \| \ | |
   / _ \| | | | | || | | | |\/| | / _ \ | || | | |  \| |
  / ___ \ |_| | | || |_| | |  | |/ ___ \| || |_| | |\  |
 /_/   \_\___/  |_| \___/|_|  |_/_/   \_\_| \___/|_| \_|
   ____ ___  ____  _____                  ___   ___   _ 
  / ___/ _ \|  _ \| ____|         __   __/ _ \ / _ \ / |
 | |  | | | | |_) |  _|           \ \ / / | | | | | || |
 | |__| |_| |  _ <| |___           \ V /| |_| | |_| || |
  \____\___/|_| \_\_____|           \_/  \___(_)___(_)_|

   ▄▀▀▀█ █ █ █ ▀▀█▀▀ ▄▀▀▀█ █▀▄▀█ ▄▀▀▀█ ▀▀█▀▀ ▄▀▀▀█ █▀█ █
   █▀▀▀█ █ ▀ █ █ █ █ █ ▀ █ █ ▀ █ █▀▀▀█ █ █ █ █ ▀ █ █ █ █
   ▀ ▀ ▀  ▀▀▀  ▀ ▀ ▀ ▀▀▀▀  ▀ ▀ ▀ ▀ ▀ ▀ ▀ ▀ ▀ ▀▀▀▀  ▀ ▀▀▀

   ▄▀▀▀█ ▄▀▀▀█ █▀▀▀▄ █▀▀▀▀           ▄▀▀▀█   ▄▀▀▀█   ▄█ 
   █ ▀ ▄ █ ▀ █ █▀▀▀▄ █▀▀▀            █ ▀ █   █ ▀ █    █   
   ▀▀▀▀  ▀▀▀▀  ▀   ▀ ▀▀▀▀▀           ▀▀▀▀  ▀ ▀▀▀▀  ▀ ▀▀▀ 

   █▀▀▀▀ █▀▀▀█ █▀█ ▀ █▀▀▀▀
   █ ▀▀▀ █ ▀ █ █▀▀▀█ █▀▀ ▀
   ▀▀▀▀▀ ▀▀▀▀▀ ▀ ▀ ▀ ▀▀▀▀▀
   █▀▀▀█ █ █▀█ █▀▀▀█ █ ▀ █ █▀▀▀▀ █▀█ ▀ █▀▀▀█ █ █ █ █▀█ █ █▀▀▀█
   █▀▀▀▀ █ ▀▀▀ █▀▀▀█ ▀▀█▀▀ █ ▀▀█ █▀▀▀█ █ ▀ █ █ ▀ █ █ █ █ █ ▀ █
   ▀ ▀▀▀ ▀▀▀▀▀ ▀ ▀ ▀   ▀   ▀▀▀▀▀ ▀ ▀ ▀ ▀▀▀▀▀ ▀▀▀▀▀ ▀ ▀▀▀ ▀ ▀▀▀

*/

static const char* automaton_ascii_logo = R"(

   █▀▀▀█ █ █ █ ▀▀█▀▀ █▀▀▀█ █▀█▀█ █▀▀▀█ ▀▀█▀▀ █▀▀▀█ █▀█ █ 
   █▀▀▀█ █ ▀ █ █ █ █ █ ▀ █ █ ▀ █ █▀▀▀█ █ █ █ █ ▀ █ █ █ █ CORE
   ▀ ▀ ▀ ▀▀▀▀▀ ▀ ▀ ▀ ▀▀▀▀▀ ▀ ▀ ▀ ▀ ▀ ▀ ▀ ▀ ▀ ▀▀▀▀▀ ▀ ▀▀▀ v0.0.1

These are common Automaton commands used in various situations:

   modules    Show list of registered modules
   protos     Show list of registered smart protocol definitions
   nodes      Show list of node instances running on this client
   launch     Launch a smart protocol node instance from a definiition
   use        Set the current smart protocol node
   msg        Construct and send a message to the current smart protocol
)";

int main(int argc, char* argv[]) {
  std::cout << automaton_ascii_logo;
  while (1) {
    std::string command;
    if (std::cin.eof()) {
      std::cout << std::endl;
      break;
    }
    std::cout << "|A| ";
    std::getline(std::cin, command);
  }
  return 0;
}
