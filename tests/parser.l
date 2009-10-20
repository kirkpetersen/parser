# parser written in my new language

type parser {
    productions : map of ( string, list of vector of symbol )
    state_stack : stack 
}
