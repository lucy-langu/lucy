if exists("b:current_syntax") | finish | endif
syntax keyword lucyKeyword if unless else while repeat for foreach loop switch case default func lambda class struct return break continue try catch finally throw import from as in let var const global new self super
syntax keyword lucyStorage const global new self super
syntax keyword lucyBoolean true false nil
syntax keyword lucyConstant PI E VERSION PLATFORM
syntax match lucyComment /#.*$/
syntax region lucyBlockComment start=/^=begin\>/ end=/^=end\>/
syntax region lucyString start=/"/ skip=/\\./ end=/"/ contains=lucyInterpolation
syntax region lucyString start=/'/ skip=/\\./ end=/'/ contains=lucyInterpolation
syntax match lucyInterpolation /\$[A-Za-z_][A-Za-z0-9_.]*/ contained
syntax region lucyShell start=/`/ end=/`/
syntax match lucyNumber /\<\(0[xX][0-9A-Fa-f]\+\|[0-9]\+\%([.][0-9]\+\)\?\)\>/
syntax match lucyOperator /\*\*=?\|===\?\|!==\?\|==\?\|!=\|>=\?\|<=\?\|&&\|||\|<<=?\|>>=?\|[+*/%&|^~!?:=<>-]=\?/
syntax match lucyFunction /\<func\s\+\zs[A-Za-z_][A-Za-z0-9_]*/
highlight default link lucyKeyword Keyword
highlight default link lucyStorage StorageClass
highlight default link lucyBoolean Boolean
highlight default link lucyConstant Constant
highlight default link lucyComment Comment
highlight default link lucyBlockComment Comment
highlight default link lucyString String
highlight default link lucyInterpolation Identifier
highlight default link lucyShell String
highlight default link lucyNumber Number
highlight default link lucyOperator Operator
highlight default link lucyFunction Function
let b:current_syntax = 'lucy'
