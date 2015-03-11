
%%{
	machine comment;

	comment = "'" (^'\n')*
	        | ("{{" (any* :>> "}}"))
	        | ("{" (^"{") @{fhold; /*std::cout << "+codecomment\n";*/ fcall codecomment;});




	blockcomment = "{"  @{/*std::cout << "+codecomment\n";*/ fcall codecomment; }
	           :>> "{{" @{/*std::cout << "+codecomment2\n";*/ fcall codecomment2;};

	codecomment  := ((any | blockcomment)* :>> "}" ) @{/*std::cout << "-codecomment\n";*/ fret;}; 
	codecomment2 := ((any | blockcomment)* :>> "}}") @{/*std::cout << "-codecomment2\n";*/ fret;};

}%%
