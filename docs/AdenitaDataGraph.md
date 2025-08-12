# Adenita Data Graph

Nodes in Adenita derive from SAMSON nodes and are described below:

`Part` (`SBStructuralModel`):

  - `ADNDoubleStrand` (`SBStructuralGroup`)

    - `ADNBaseSegment` (`SBStructuralGroup`). It has a reference to `ADNCell`. `ADNCell` might have references to right and left nucleotides or to right and left loops.

  - `ADNSingleStrand` (`SBChain`)

    - `ADNNucleotide` (`SBResidue`). It has references to a pair nucleotide and to the ADNBaseSegment to which it belongs.

      - `ADNBackbone` (`SBBackbone`)

        - `ADNAtom` (`SBAtom`)

      - `ADNSidechain` (`SBSideChain`)

        - `ADNAtom` (`SBAtom`)

`ADNCell` might have references to right and left nucleotides or right and left loops.

`ADNLoop` has a list of references to nucleotides.

Aux. reference nodes (like atoms in bonds) to connect single and double strands, base segments and nucleotides.
