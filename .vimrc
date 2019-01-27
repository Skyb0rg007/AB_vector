let g:ale_c_gcc_options = '-Wall -Wextra -std=c90 -pedantic -Winline -Wconversion'
augroup AB_vector
    " this one is which you're most likely to use?
    autocmd BufRead,BufNewFile *.h set ft=c.doxygen
augroup end
