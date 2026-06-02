from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]

REQUIRED_UI_FILES = [
    "SEConnectSSDNAEditorGUI.ui",
    "SETwistHelixEditorGUI.ui",
    "SEDNATwisterEditorGUI.ui",
    "SETaggingEditorGUI.ui",
    "SEBreakEditorGUI.ui",
    "SEDSDNACreatorEditorGUI.ui",
    "SEMergePartsEditorGUI.ui",
    "SELatticeCreatorEditorGUI.ui",
    "SENanotubeCreatorEditorGUI.ui",
    "SEWireframeEditorGUI.ui",
    "SEAdenitaCoreSettings.ui",
]

REQUIRED_SETUP = {
    "SEConnectSSDNAEditorGUI.cpp": [
        "setupHelpText",
        "updateSequenceControls",
        "radioButtonSingleStrand",
        "chkConcat",
        "lineSequence",
        "checkBoxAutoFill",
    ],
    "SETwistHelixEditorGUI.cpp": [
        "setupHelpText",
        "radioButtonMinusBP",
        "radioButtonPlusBP",
        "doubleSpinBoxAngle",
        "spinBoxTurns",
    ],
    "SEDNATwisterEditorGUI.cpp": [
        "setupHelpText",
        "rdbUntwist",
        "rdbInvisible",
        "btnResetUntwist->setVisible(false)",
        "btnResetInvisible->setVisible(false)",
    ],
    "SETaggingEditorGUI.cpp": [
        "setupHelpText",
        "radioButtonTags",
        "radioButtonBase",
        "comboBoxTargetBase",
        "onTargetBaseChanged",
        "setTargetBaseFromEditor",
    ],
    "SEBreakEditorGUI.cpp": ["setupHelpText", "radioButtonThreePrime", "radioButtonFivePrime"],
    "SEDSDNACreatorEditorGUI.cpp": ["setupHelpText", "radioButtonSSDNA", "radioButtonDSDNA", "checkBoxManual"],
    "SEMergePartsEditorGUI.cpp": ["setupHelpText", "pushButtonRefresh", "comboBoxMergeComponent1", "comboBoxMoveElement"],
    "SELatticeCreatorEditorGUI.cpp": ["setupHelpText", "radioButtonSquareLattice", "radioButtonHoneycombLattice", "spinBoxMaxZbp"],
    "SENanotubeCreatorEditorGUI.cpp": ["setupHelpText", "comboBoxRouting", "doubleSpinBoxRadius", "spinBoxNumberOfBasePairs"],
    "SEWireframeEditorGUI.cpp": ["setupHelpText", "radioButtonTetrahedron", "radioButtonTruncated_cuboctahedron"],
    "SEAdenitaCoreSettingsGUI.cpp": ["setupHelpText", "comboBoxScaffold", "lineEditNtthal", "checkBoxGenerateAtomicDetails"],
}


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def require_tokens(path: Path, tokens: list[str]) -> list[str]:
    text = read(path)
    return [token for token in tokens if token not in text]


def main() -> int:
    failures: list[str] = []

    form_dir = ROOT / "AdenitaCoreSE" / "form"
    for file_name in REQUIRED_UI_FILES:
        if not (form_dir / file_name).exists():
            failures.append(f"missing UI file: {file_name}")

    source_dir = ROOT / "AdenitaCoreSE" / "source"
    for file_name, tokens in REQUIRED_SETUP.items():
        path = source_dir / file_name
        if not path.exists():
            failures.append(f"missing source file: {file_name}")
            continue
        missing = require_tokens(path, tokens)
        if missing:
            failures.append(f"{file_name}: missing {missing}")

    tagging_ui = read(form_dir / "SETaggingEditorGUI.ui")
    for token in ["comboBoxTargetBase", "Target base:", "Mutate base"]:
        if token not in tagging_ui:
            failures.append(f"SETaggingEditorGUI.ui: missing {token}")

    tagging_header = read(ROOT / "AdenitaCoreSE" / "include" / "SETaggingEditor.hpp")
    for token in ["setNucleotideType(DNABlocks type)", "getNucleotideType() const"]:
        if token not in tagging_header:
            failures.append(f"SETaggingEditor.hpp: missing {token}")

    if failures:
        print("UI help text checks failed:")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("UI help text checks passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
