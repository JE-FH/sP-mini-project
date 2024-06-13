#!/usr/bin/env bash

set -ex

function make_latex {
	author=$1
	cat <<'EOF'
\input{./base.tex}
EOF
	echo '\title{'$title'}'
	echo '\author{'$author'}'
	echo '\begin{document}'
	echo '  \maketitle'

	tex_files=$(find . -type f -iname "*.tex")
	for f in $tex_files ; do
		filename=$(basename $f)
		case "$filename" in
			"listing.tex" | "base.tex")
				continue
				;;
		esac
		echo '  \input{'$f'}'
	done
	header_files=$(find ../sP-mini-project -type f \( -iname "*.h" -o -iname "*.hh" -o -iname "*.hpp" \))
	source_files=$(find ../sP-mini-project -type f \( -iname "*.c" -o -iname "*.cc" -o -iname "*.cpp" \))
	for f in $header_files $source_files; do
		dirname=${f#./}
		dirname=${dirname%%/*}
		case "$dirname" in
			.idea|"cmake-build-"*|"build"|"out")
				continue
				;;
		esac
		filename=$(basename $f)
		case "$filename" in
			"doctest.h")
				continue
				;;
		esac
		name=$(echo "$f" | sed -e 's/_/\\_/g')
		echo '  \lstinputlisting[style=colorC++,caption={'$name'}]{'$f'}'
	done
	for f in "../CMakeLists.txt"; do
		dirname=${f#./}
		dirname=${dirname%%/*}
		case "$dirname" in
			.idea|"cmake-build-"*|"build"|"out")
				continue
				;;
		esac
		filename=$(basename $f)
		name=$(echo "$f" | sed -e 's/_/\\_/g')
		echo '  \lstinputlisting[style=colorBash,caption={'$name'}]{'$f'}'
	done

	echo '\end{document}'
}

author="Jens Emil Fink Højriis"
title="Sp mini project"

inkscape "../out/build/x64-release/sP-mini-project/Circadian rhythm.svg" -o "media/Circadian rhythm.png"
inkscape "../out/build/x64-release/sP-mini-project/Covid 19.svg" -o "media/Covid 19.png"
inkscape "../out/build/x64-release/sP-mini-project/Figure 1.svg" -o "media/Figure 1.png"	
dot -Tpng "../out/build/x64-release/sP-mini-project/covid19.dot" > "media/covid19 reaction graph.png"

echo "Generates LaTeX report from C/C++ sources and TeX files it can find in the current directory."
echo "Assumes that pdflatex (from TeX Live) is installed."
echo -e "Usage:\n\t$0 \"Author Name\" \"Document Title\""
make_latex "$author" "$title" > listing.tex
pdflatex --enable-write18 -interaction=nonstopmode listing.tex && pdflatex --enable-write18 -interaction=nonstopmode listing.tex && okular listing.pdf
