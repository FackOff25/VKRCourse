%%
%% Tsinghua Science and Technology (Tsinghua Sci Technol) - LaTeX Class File
%%
%% Copyright (c) 2025 latexstudio, website <www.latexstudio.net>, email <typeset@latexstudio.net>
%%
%% Author      : latexstudio
%% Website     : www.latexstudio.net
%% Email       : typeset@latexstudio.net
%% Date        : 2025-09-11 11:20:20
%% Description : Tsinghua Science and Technology (Tsinghua Sci Technol) - LaTeX Class File
%%
%% This work may be distributed and/or modified under the terms of the
%% LaTeX Project Public License (LPPL), version 1.3c or later.
%% The latest version of this license is available at:
%%    http://www.latex-project.org/lppl.txt
%%
%% This work is maintained by latexstudio.
%%
\NeedsTeXFormat{LaTeX2e}[1999/12/01]
\ProvidesClass{TST}[2025/09/11, v1.0 by latexstudio.net]

\newif\if@linenumberon\global\@linenumberonfalse%
\DeclareOption{lineno}{\global\@linenumberontrue}%

\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
\ProcessOptions\relax
\LoadClass[twoside,twocolumn,10pt]{article}

\RequirePackage{geometry}
\geometry{
  papersize  = {216mm,285mm},
  textheight = 234mm,
  left       = 21mm,
  right      = 21mm,
  top        = 27mm,
  columnsep  = 5.5mm,
  footskip   = 9mm,
  bottom     = 23mm,
  headsep    = 17pt,
  % showframe
}

\RequirePackage[expansion=true,protrusion=false]{microtype}
\RequirePackage[ruled,longend,noline]{algorithm2e}
\RequirePackage[numbers,sort&compress]{natbib}
\RequirePackage[explicit,clearempty,newparttoc]{titlesec}
\RequirePackage{
amsmath       , amssymb   , bm     , booktabs , caption   , enumitem ,
etoolbox      , fancyhdr  , framed , graphicx , makecell  , marvosym ,
multirow      , newtxtext , pifont , siunitx  , subfigure , tikz     ,
tikzpagenodes , upgreek   , url    , diagbox
}
\let\Cross\relax
\RequirePackage{bbding}

\RequirePackage[mathlines,running,switch]{lineno}

\if@linenumberon%
\let\linenumbershook\linenumbers
\else
\let\linenumbershook\empty
\fi%

\hyphenchar\font=-1
\newcolumntype{P}[1]{p{#1}<{\raggedright}}
\sfcode`.=1200
\sfcode`:=1200
\def\lk{\linebreak}

\captionsetup{
  labelfont       = {bf},
  font            = {small},
  labelsep        = quad,
  aboveskip       = 8pt,
  singlelinecheck = true,
}
\captionsetup[figure]{
  name      = {Fig.},
  aboveskip = 8pt,
  belowskip = -3pt,
}
\captionsetup[table]{
  name      = {Table},
  skip      = 1pt,
  belowskip = 1pt,
}

\usetikzlibrary{calc}
\graphicspath{{figures/}}
\allowdisplaybreaks
\SetArgSty{textnormal}
\SetAlgoCaptionSeparator{~~~}
\patchcmd{\@algocf@pre@ruled}{\algoheightrule}{.8pt}{}{}
\algotitleheightrule .4pt
\algoheightrule .8pt
\aboverulesep 1pt
\belowrulesep 1pt
\urlstyle{same}
% \definecolor{main}{HTML}{7A2A87}
\colorlet{main}{black}
\setlist{nosep}
\setlist[enumerate,1]{label=(\arabic*)}

\def\le{\leqslant}
\def\ge{\geqslant}
\def\leq{\leqslant}
\def\geq{\geqslant}
\def\etal{\textit{et~al}.}

\DeclareMathOperator{\can}{can}
\DeclareMathOperator{\argmin}{arg\,min}
\DeclareMathOperator{\diag}{diag}

\let\ph\phantom
\newcommand{\cor}{\textsuperscript{\rm (\Letter)}}
\newcommand{\note}[1]{{\normalsize\normalfont\color{blue}#1}}
\newcommand{\tabnote}[2][\linewidth]{\parbox{#1}{\footnotesize #2}}
\renewcommand{\@biblabel}[1]{[#1]\hfill}
\renewcommand{\thesubfigure}{(\alph{subfigure})}
\renewcommand{\refname}{\fontsize{10.5pt}{10.5pt}\selectfont \textbf{Reference}}
\renewcommand{\bibfont}{\fontsize{9pt}{12.8pt}\selectfont }
\@addtoreset{footnote}{page}
\AtBeginEnvironment{thebibliography}{\bigskip}

\bibsep           0pt plus .3pt minus .7pt
\abovedisplayskip 4pt plus .3pt minus .7pt
\belowdisplayskip 4pt plus .3pt minus .7pt
\abovedisplayshortskip 7pt
\belowdisplayshortskip 7pt
\floatsep         9pt plus 2pt minus 2pt
\textfloatsep     11pt plus 2pt minus 2pt
\parskip          0pt plus .2pt minus .2pt
\partopsep        3pt plus 1pt minus 1pt

\NewDocumentCommand\@titleinfo{mmm}{
  \begin{tikzpicture}[remember picture,overlay]
  \coordinate (O) at (current page header area.south west);
  \coordinate (P) at (current page header area.south east);
  \draw[main,line width=.8pt] (O) -- (P);
  \node[anchor = south west,inner sep=0pt] at ([yshift=3pt]O) {\makecell[l]{{\itshape\bfseries #1}\\ Doi: #3} };
  \node[anchor = north east,inner sep=0pt] at ([yshift=-3pt]P) {#2};
  \end{tikzpicture}
}

\ExplSyntaxOn

\keys_define:nn { TSTkeys } {
  type      . tl_set:N  = \l_TSTkeys_type_tl,
  type      . initial:n = (Research/Review)~Article,
  name      . tl_set:N  = \l_TSTkeys_name_tl,
  name      . initial:n = Tsinghua~ Science~ and~ Technology,
  doi       . tl_set:N  = \l_TSTkeys_doi_tl,
  doi       . initial:n = {s41095-0xx-xxxx-x},
  title     . tl_set:N  = \l_TSTkeys_title_tl,
  title     . initial:n = title content,
  author    . tl_set:N  = \l_TSTkeys_author_tl,
  % author    . initial:n = author,
  address   . clist_set:N  = \l_TSTkeys_address_clist,
  EditorNote . clist_set:N  = \l_TSTkeys_EditorNote_clist,
  runauthor . tl_set:N  = \l_TSTkeys_runauthor_tl,
  % runauthor . initial:n = runauthor,
  runtitle  . tl_set:N  = \l_TSTkeys_runtitle_tl,
  runtitle  . initial:n = \l_TSTkeys_title_tl,
  abstract  . tl_set:N  = \l_TSTkeys_abstract_tl,
  abstract  . initial:n = abstract,
  keywords  . tl_set:N  = \l_TSTkeys_keywords_tl,
  keywords  . initial:n = keywords,
  logo      . tl_set:N  = \l_TSTkeys_logo_tl,
  logo      . initial:n = logo,
  received  . tl_set:N  = \l_TSTkeys_received_tl,
  received  . initial:n = 20xx-xx-xx,
  revised   . tl_set:N  = \l_TSTkeys_revised_tl,
  revised   . initial:n = 20xx-xx-xx,
  accepted  . tl_set:N  = \l_TSTkeys_accepted_tl,
  accepted  . initial:n = 20xx-xx-xx,
  copyright . tl_set:N  = \l_TSTkeys_copyright_tl,
  % copyright . initial:n = {\copyright{}~Tsinghua~University~Press}
}

\NewDocumentCommand\TSTsetup{ +m }{
  \keys_set:nn{TSTkeys}{#1}
}

\fancypagestyle{plain}{
  \fancyhf{}
  % \renewcommand{\headrulewidth}{1pt}
  \fancyfoot[LE,RO]{\includegraphics[width=125pt]{\l_TSTkeys_logo_tl}}
}

\fancypagestyle{main}{
  \fancyhf{}
  \fancyhead[EL,OR]{\thepage}
  \fancyfoot[LE,RO]{\includegraphics[width=125pt]{\l_TSTkeys_logo_tl}}
  \fancyhead[ER]{\l_TSTkeys_runauthor_tl}
  \fancyhead[OL]{\l_TSTkeys_runtitle_tl}
}

\DeclareDocumentCommand\address{m}{
\clist_gput_right:Nn \l_TSTkeys_address_clist {{#1}}
}

\DeclareDocumentCommand\EditorNote{m}{
\clist_gput_right:Nn \l_TSTkeys_EditorNote_clist {{#1}}
}

\def\headrule{{\if@fancyplain\let\headrulewidth\plainheadrulewidth\fi\color{main}
    \hrule\@height\headrulewidth\@width\headwidth
    \vskip-\headrulewidth}}

\AtBeginEnvironment{tabular}{\small}
\AtBeginEnvironment{tabular*}{\small}

\AtBeginDocument{
  \baselineskip 14pt plus .2pt minus .2pt
    \renewcommand\footnoterule{%
    \kern-3\p@
    \hrule\@width \columnwidth
    \kern2.6\p@}
  \renewcommand{\@maketitle}{%
    \label{FirstPage}
    % \thispagestyle{plain}
    \@titleinfo{\l_TSTkeys_name_tl}{\l_TSTkeys_type_tl}{\l_TSTkeys_doi_tl}\par
    \vspace*{.5cm}
    \parindent 0pt
    {\bfseries\fontsize{15pt}{15pt}\selectfont\color{main} \l_TSTkeys_title_tl\par}
    \vspace*{.8cm}
    {\bfseries\fontsize{10.5pt}{10.5pt}\selectfont \l_TSTkeys_author_tl\par}
    \tl_if_empty:NF \l_TSTkeys_copyright_tl {
    \vspace*{.6cm}
    \textcopyright{}~\l_TSTkeys_copyright_tl{}\par
    }
    \vspace*{2em}
  }
  \DeclareDocumentCommand\maketitle{}{
  \linenumbershook
  \begingroup
  \parindent 0pt
    \renewcommand\thefootnote{\@arabic\c@footnote}%
    \newskip\@footindent
    \@footindent=.6em
    \thispagestyle{plain}
    \def\@makefnmark{\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%
    % \long\def\@makefntext##1{\parindent 1em\noindent
    %         {{\normalfont\@thefnmark}}\quad ##1}%
    \long\def\@makefntext##1{\@setpar{\@@par\@tempdima \hsize
    \advance\@tempdima-\@footindent
    \parshape \@ne \@footindent \@tempdima}\par
    \noindent \hbox to \z@{\hss\@thefnmark\hspace{0.2em}}##1}
    \twocolumn[\@maketitle]%
    \textbf{\color{main}Abstract} \quad \l_TSTkeys_abstract_tl{}\par
    \bigskip
    \textbf{\color{main}Keywords} \quad \l_TSTkeys_keywords_tl{}\par
    \bigskip
   \renewcommand\thefootnote{\textbullet}
   \int_set:Nn \l_tmpa_int {\clist_count:N \l_TSTkeys_address_clist}
   \int_step_inline:nn {\l_tmpa_int}{
   \footnotetext[##1]{\clist_item:Nn \l_TSTkeys_address_clist {##1}}
   }
   \renewcommand\thefootnote{}
   \int_set:Nn \l_tmpb_int {\clist_count:N \l_TSTkeys_EditorNote_clist}
   \int_step_inline:nn {\l_tmpb_int}{
   \footnotetext{\clist_item:Nn \l_TSTkeys_EditorNote_clist {##1}}
   }
   \footnotetext{Manuscript~ received:~ \l_TSTkeys_received_tl;~ revised:~ \l_TSTkeys_revised_tl;~ accepted:~ \l_TSTkeys_accepted_tl\par}
  \endgroup
  \setcounter{footnote}{0}%
  \pagestyle{main}
  \global\let\thanks\relax
  \global\let\maketitle\relax
  \global\let\@maketitle\relax
  \global\let\@thanks\@empty
  \global\let\@author\@empty
  \global\let\@date\@empty
  \global\let\@title\@empty
  \global\let\title\relax
  \global\let\author\relax
  \global\let\date\relax
  \global\let\and\relax
}
}

\ExplSyntaxOff

\setcounter{secnumdepth}{3}

\titleformat{\section}{\color{main}\bfseries\fontsize{12pt}{14pt}\selectfont}
{\thesection}{1em}{#1}
\titleformat{\subsection}{\bfseries\fontsize{10.5pt}{13.5pt}\selectfont}
{\thesubsection}{.8em}{#1}
\titleformat{\subsubsection}{\fontsize{10.5pt}{10.5pt}\selectfont \itshape}
{\thesubsubsection}{.8em}{#1}

\titlespacing*{\section}{0pt}{.7em  plus .3ex minus .2ex}{.3em  plus .2ex}
\titlespacing*{\subsection}{0pt}{1em  plus 1ex minus .2ex}{.5em  plus .2ex}
\titlespacing*{\subsubsection}{0pt}{.6em  plus 1ex minus .2ex}{.3em  plus .2ex}

\renewcommand\appendix{\par
  \setcounter{section}{0}%
  \gdef\thesection{\@Alph\c@section}
  \titleformat{\subsection}{\bfseries\color{main}\fontsize{10.5pt}{10.5pt}\selectfont}
  {\thesubsection}{.8em}{##1}
  }

\def\UrlBreaks{%
  \do\/%
  \do\a\do\b\do\c\do\d\do\e\do\f\do\g\do\h\do\i\do\j\do\k\do\l%
     \do\m\do\n\do\o\do\p\do\q\do\r\do\s\do\t\do\u\do\v\do\w\do\x\do\y\do\z%
  \do\A\do\B\do\C\do\D\do\E\do\F\do\G\do\H\do\I\do\J\do\K\do\L%
     \do\M\do\N\do\O\do\P\do\Q\do\R\do\S\do\T\do\U\do\V\do\W\do\X\do\Y\do\Z%
  \do0\do1\do2\do3\do4\do5\do6\do7\do8\do9\do=\do/\do.\do:%
  \do\*\do\-\do\~\do\'\do\"\do\-}
\Urlmuskip=0mu plus 0.1mu

\def\normalsize{\@setsize\normalsize{12pt}\xpt\@xpt
  \baselineskip 14pt plus.75pt minus.75pt%
}
\def\footnotesize{\@setsize\footnotesize{9.5pt}\viiipt\@viiipt
  \baselineskip=10pt plus.25pt minus.25pt%
}

\newskip\@footindent
\@footindent=.8em
\renewcommand\footnoterule{\kern-3\p@ \hrule width 0.4\columnwidth \kern 2.6\p@}
\long\def\@makefntext#1{\@setpar{\@@par\@tempdima \hsize
\advance\@tempdima-\@footindent
\parshape \@ne \@footindent \@tempdima}\par
\noindent \hbox to \z@{\hss\@thefnmark\hspace{0.2em}}#1}
\renewcommand\thefootnote{\myfootnotestyle{\arabic{footnote}}}
\def\@makefnmark{\hbox{\textsuperscript{\@thefnmark}}}
\newcommand\myfootnotestyle[1]{\ifcase#1 \or \ding{172}\or \ding{173}\or
  \ding{174}\or \ding{175}\or \ding{176}\or \ding{177}%
  \or \ding{178}\or \ding{179}\or \ding{180}\or \ding{181}\else *\fi\relax}

% MGStran class scratch pad registers
% dimen
\newdimen\@TSTtrantmpdimenA
\newdimen\@TSTtrantmpdimenB
% count
\newcount\@TSTtrantmpcountA
\newcount\@TSTtrantmpcountB
% token list
\newtoks\@TSTtrantmptoksA

\def\@TSTtranneedspace#1#2{\penalty-100\begingroup%shield temp variable
\@TSTtrantmpdimenB\pagegoal\advance\@TSTtrantmpdimenB-\pagetotal% space left
\ifdim #1>\@TSTtrantmpdimenB\relax% not enough space left
\ifdim\@TSTtrantmpdimenB>\z@\relax #2\fi%
\newpage%
\fi\endgroup}
\begingroup
\catcode`\Q=3
\long\gdef\@ifmtarg#1{\@xifmtarg#1QQ\@secondoftwo\@firstoftwo\@nil}
\long\gdef\@xifmtarg#1#2Q#3#4#5\@nil{#4}
\endgroup

\newcounter{biography}
\setcounter{biography}{0}

% photo area size
\def\@TSTBIOphotowidth{1in}  % width of the biography photo area
\def\@TSTBIOphotodepth{1.25in}  % depth (height) of the biography photo area
% area cleared for photo
\def\@TSTBIOhangwidth{1.1in}  % width cleared for the biography photo area
\def\@TSTBIOhangdepth{1.25in}  % depth cleared for the biography photo area
                 % actual depth will be a multiple of
                 % \baselineskip, rounded up
\def\@TSTBIOskipN{2\baselineskip}% nominal value of the vskip above the biography

\newenvironment{biography}[2][]{\normalfont %
\unitlength 1in\parskip=0pt\par\parindent 1em\interlinepenalty500%
% we need enough space to support the hanging indent
% the nominal value of the spacer
% and one extra line for good measure
\@TSTtrantmpdimenA=\@TSTBIOhangdepth%
\advance\@TSTtrantmpdimenA by \@TSTBIOskipN%
\advance\@TSTtrantmpdimenA by 1\baselineskip%
% if this page does not have enough space, break it and lets start
% with a new one
\@TSTtranneedspace{\@TSTtrantmpdimenA}{\relax}%
% nominal spacer can strech, not shrink use 1fil so user can out stretch with \vfill
% \vskip \@TSTBIOskipN plus 1fil minus 0\baselineskip%
% the default box for where the photo goes
\def\@TSTtempbiographybox{{\setlength{\fboxsep}{0pt}\framebox{%
\begin{minipage}[b][\@TSTBIOphotodepth][c]{\@TSTBIOphotowidth}\centering PLACE\\ PHOTO\\ HERE \end{minipage}}}}%
%
% detect if the optional argument was supplied, this requires the
% \@ifmtarg command as defined in the appendix section above
% and if so, override the default box with what they want
\@ifmtarg{#1}{\relax}{\def\@TSTtempbiographybox{\mbox{\begin{minipage}[b][\@TSTBIOphotodepth][c]{\@TSTBIOphotowidth}%
\centering%
\includegraphics[width=25mm]{#1}%
\end{minipage}}}}% end if optional argument supplied
% Make an entry into the table of contents only if we have not done so before
%\if@biographyTOCentrynotmade%
%% link labels to the biography counter so hyperref will jump
%% to the biography, not the previous section
%\setcounter{biography}{-1}%
%\refstepcounter{biography}%
%\addcontentsline{toc}{section}{Biographies}%
%\global\@biographyTOCentrynotmadefalse%
%\fi%
% one more biography
\refstepcounter{biography}%
% Make an entry for this name into the table of contents
% \addcontentsline{toc}{subsection}{#2}%
% V1.6 properly handle if a new paragraph should occur while the
% hanging indent is still active. Do this by redefining \par so
% that it will not start a new paragraph. (But it will appear to the
% user as if it did.) Also, strip any leading pars, newlines, or spaces.
\let\@TSTBIOORGparCMD=\par% save the original \par command
\edef\par{\hfil\break\indent}% the new \par will not be a "real" \par
\settoheight{\@TSTtrantmpdimenA}{\@TSTtempbiographybox}% get height of biography box
\@TSTtrantmpdimenB=\@TSTBIOhangdepth%
\@TSTtrantmpcountA=\@TSTtrantmpdimenB% countA has the hang depth
\divide\@TSTtrantmpcountA by \baselineskip%  calculates lines needed to produce the hang depth
\advance\@TSTtrantmpcountA by 1% ensure we overestimate
% set the hanging indent
\hangindent\@TSTBIOhangwidth%
\hangafter-\@TSTtrantmpcountA%
% reference the top of the photo area to the top of a capital T
\settoheight{\@TSTtrantmpdimenB}{\mbox{T}}%
% set the photo box, give it zero width and height so as not to disturb anything
\noindent\makebox[0pt][l]{\hspace{-\@TSTBIOhangwidth}\raisebox{\@TSTtrantmpdimenB}[0pt][0pt]{%
\raisebox{-\@TSTBIOphotodepth}[0pt][0pt]{\@TSTtempbiographybox}}}%
% now place the author name and begin the bio text
\fontsize{9pt}{12.8pt}\selectfont  \noindent{\textbf{#2}\ }}{\relax\let\par=\@TSTBIOORGparCMD\par%
% 7/2001 V1.5 detect when the biography text is shorter than the photo area
% and pad the unused area - preventing a collision from the next biography entry
% MDS
\ifnum \prevgraf <\@TSTtrantmpcountA\relax% detect when the biography text is shorter than the photo
    \advance\@TSTtrantmpcountA by -\prevgraf% calculate how many lines we need to pad
    \advance\@TSTtrantmpcountA by -1\relax% we compensate for the fact that we indented an extra line
    \@TSTtrantmpdimenA=\baselineskip% calculate the length of the padding
    \multiply\@TSTtrantmpdimenA by \@TSTtrantmpcountA%
    \noindent\rule{0pt}{\@TSTtrantmpdimenA}% insert an invisible support strut
\fi%
\par\normalfont}

\AtEndPreamble{
  \RequirePackage{hyperref}
  \hypersetup{
  CJKbookmarks       = true,
  bookmarksnumbered  = true,
  bookmarksopen      = true,
  bookmarksopenlevel = 2,
  unicode            = true,
  colorlinks         = true,
  breaklinks         = false,
  pdfsubject         = {www.latexstudio.net},
  }
}
\AtEndDocument{\label{LastPage}}

\endinput

%%
%% end TST.cls