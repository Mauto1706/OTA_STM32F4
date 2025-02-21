#Directory Structure
```
/repo
    +--doc
    |   +--design
    |   |   +--<component>_Swc_Architecture_Design.docx
    |   |   +--<component>_Swc_Detail_Design.docx
    |   +--reports
    |   |   +--review_checklist
    |   |   	+--<component>_CodeReview.xlsx
    |   |   	+--<component>_DesignReview.xlsx
    |   |   +--<component>_TraceAbilityMatrix.xlsx
    |   |   +--<component>_RequirementAnalysis.xlsx    (link between component with product/software requirements)
    |   +--user_manual
    |   |   +--<component>_UserManual.chm              (generate with doxygen)
    +--source
        +--cfg
            +--<*_cfg.h>
            +--<*_cfg.c>
        +--<*.h>
        +--<*.c>
```
#Guidelines
Retrieve the entire directory
```
git clone --recursive http://itc-server.dynu.net:7990/scm/itd/w25n01gv.git
```
