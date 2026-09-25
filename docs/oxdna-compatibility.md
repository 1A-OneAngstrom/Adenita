# oxDNA compatibility

Adenita reads and writes the classic oxDNA topology format. Topology and
configuration records use the same nucleotide indices. Standard exports list
nucleotides from 3' to 5', with neighbor columns in that order, and use oxDNA
length units (one unit is 0.8518 nm). Box sizes in the export dialog remain in nm.

Import validates connectivity and configuration records before adding a model.
Linear and circular DNA are supported, including records whose order differs
from strand traversal. Configuration records may contain nine fields, or fifteen
when velocities and angular velocities are present. Unsupported topology formats
and base types produce an error instead of being silently converted.

Select **Older Adenita export** when importing files written by earlier Adenita
versions. These files used the opposite neighbor-column interpretation, positions
in nm, and a different orientation mapping. The option reverses those old export
conventions. Old files have no reliable identifying marker, so their origin
cannot be determined automatically. Leave the option unchecked for standard
oxDNA files, including exports from current Adenita.

The standard frame conversion maps the base direction to Adenita's inward e2
axis and the 3'-to-5' base normal to -e3. The legacy conversion reverses the old
export mapping, a1 = -e2 and a3 = -e1.

References: [oxDNA file formats](https://lorenzo-rovigatti.github.io/oxDNA/configurations.html)
and the [reference PDB converter](https://github.com/lorenzo-rovigatti/tacoxDNA/blob/master/src/libs/pdb.py).
