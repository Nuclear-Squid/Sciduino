// A4 article layout template for Typst v0.13+ with Pandoc v3.6+.
// Adapted from John Maxwell, jmax@sfu.ca, VERSION as of March 2025
// Assumes Markdown source with a YAML metadata block (title, author, date...).
//
// Usage:
//    pandoc article.md \
//      -f markdown --wrap=none \
//      -t pdf --pdf-engine=typst \
//      -V template=article.typ \
//      -o article.pdf

// This bit from Pandoc, to help parse incoming metadata
#let content-to-string(content) = {
  if content.has("text") {
    content.text
  } else if content.has("children") {
    content.children.map(content-to-string).join("")
  } else if content.has("body") {
    content-to-string(content.body)
  } else if content == [ ] {
    " "
  }
}

#let conf(
  // these first few come through from markdown metadata:
  title: none,
  subtitle: none,
  authors: (),
  keywords: (),
  date: none,
  abstract: none,
  // the rest is defined locally here:
  comment: "Ce document est la propriété de Société Nouvelle RBI, 53 chemin du Vieux Chêne 38240 Meylan. Il est remis à titre strictement confidentiel. Il ne peut être reproduit, ni communiqué sans son autorisation expresse.",
  lang: "fr",
  region: "FR",
  paper: "a4",
  margin: (top: 3cm, bottom: 3cm, inside: 2.5cm, outside: 2.5cm),
  cols: 1,
  font: ("Liberation Sans"),
  fontsize: 12pt,
  sectionnumbering: "1.1.",
  pagenumbering: "1/1",
  doc,
) = {
  set document(
    title: title,
    author: authors.map(author => content-to-string(author.name)),
    keywords: keywords,
  )

  set page(
    paper: paper,
    margin: margin,
    numbering: pagenumbering,
    columns: cols,

    // Running header:
    header-ascent: 25% + 0pt,
    header: context {
      // show smallcaps: set text(tracking: 0.14em)
      set text(10pt)
      if (here().page()) > 1 {        // skip first page
        if calc.odd(here().page()) {  // different headers on L/R pages
          align(left, block(width: 100%)[
            #counter(page).display("1") – #title
          ])
        } else {
          align(right, block(width: 100%)[
            #authors.first().name – #counter(page).display()
            // #align(left)[#authors.first().name] #h(1fr) #align(right)[#counter(page).display("1")]
          ])
        }
      }
    },

    // Running footer
    footer-descent: 30% + 0pt,
    footer: context {
      set par(leading: 4pt)
      align(center, text(size: 9pt, style: "italic")[#comment])
    },
  )

  // Text defaults
  set text(
    lang: lang,
    region: region,
    font: font, // see 'conf' above
    size: fontsize,
    // spacing: 110%,
    alternates: false,
    discretionary-ligatures: false,
    historical-ligatures: false,
    number-type: "old-style",
    number-width: "proportional"
    //set strong(delta: 200) // use semibold instead of bold
  )

  // Paragraph defaults
  set par(
    spacing: 18pt,
    leading: 8pt,
    // fist-line-indent and justify are set below, just ahead of 'doc'
  )

  // Links
  show link: it => {
    set text(fill: rgb("#4b69c6"))
    underline(it)
  }

  // Block quotations
  set quote(block: true)
  show quote: set block(spacing: 18pt)
  show quote: set pad(x: 1.5em)
  show quote: set par(leading: 8pt)
  show quote: set text(style: "normal")

  // Code blocks: green monospace
  show raw: set block(inset: (left: 2em, top: 1em, right: 1em, bottom: 1em ))
  show raw: set text(fill: rgb("#116611"), size: 9pt)

  // Images and figures
  set image(fit: "contain")
  show image: it => {
    align(center, it)
  }
  set figure(gap: 1em, supplement: none, placement: none)
  show figure.caption: set text(size: 9pt) // how to set space below?
  show figure: set block(below: 1.5em)

  // Footnote
  set footnote.entry(indent: 0em)
  show footnote.entry: set par(spacing: 0.5em, justify: false)
  show footnote.entry: set par(hanging-indent: 0.4em) // needs work
  show footnote.entry: set text(size: 9pt, weight: 200)

  // Tables, Lists
  // show table.cell: it => {
  //   if it.y == 0 {
  //     return align(center)[#it.body]
  //   }
  //   it
  // }
  show table: set par(justify: false)
  show list: set par(justify: false)

  // HEADINGS
  //
  show heading: set text(hyphenate: false)
  set heading(numbering: sectionnumbering)

  show heading.where(level: 1): body => {
    pagebreak(weak: true)
    align(left, block(above: 48pt, below: 32pt, width: 100%)[
      #set text(weight: "bold", style: "normal", size: 1.6em)
      #v(2em)
      #body
      #v(1em)
    ])
  }

  show heading.where(level: 2): body => align(left, block(above: 32pt, below: 18pt)[
    #set text(weight: "bold", size: 1.4em)
    #body
    #v(-1em)
    #line(start: (0%, 0%), end: (100%, 0%), stroke: 0.8pt + gradient.linear(black, white))
  ])

  // STYLING LABELLED SECTIONS
  //
  show <epigraph>: set text(rgb("#777"))
  show <epigraph>: set par(justify: false)

  show <refs>: set par(
    justify: false,
    spacing: 16pt,
    first-line-indent: 0em,
    hanging-indent: 2em,
    leading: 8pt,
  )
  show <refs>: set text(black)  // for testing

  // STYLING SPECIFIC STRINGS OF TEXT
  show "LaTeX": smallcaps
  show regex("https?://\S+"): set text(style: "normal", rgb("#33d"))

  // THIS IS THE TITLE BLOCK
  v(1em)
  set par(justify: false)
  align(left, text(size: 18pt)[
    #title#if subtitle != none [: #emph[#subtitle] ]
  ])
  v(1em)
  align(left, text(size: 12pt)[#authors.first().name])
  if date != none {
    v(0.5em)
    align(left, text(size: 12pt)[#date])
  }
  if abstract != none {
    v(0.5em)
    align(left, text(size: 11pt, tracking: 0.05em)[Abstract: ]+ text(size: 11pt, style: "italic")[#abstract])
  }
  v(1em)
  //line(start: (1%,0%), end: (99%,0%), stroke: 1pt + gray)
  v(4em)

  // THIS IS THE ACTUAL BODY
  counter(page).update(1) // re-set page numbering

  set par(justify: true) //default for the rest of the doc

  doc  // HERE is the actual body content

  // COLOPHON at the end
  v(1fr)
  align(center, text(size: 9pt, style: "italic")[Fait avec Markdown + Pandoc + Typst + LanguageTool.])

} // end 'let conf'
