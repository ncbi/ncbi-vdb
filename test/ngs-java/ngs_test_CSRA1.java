import static org.junit.Assert.fail;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import org.junit.Test;

import ngs.ReadCollection;
import ngs.Alignment;
import ngs.ErrorMsg;

import gov.nih.nlm.ncbi.ngs.NGS;

//  The purpose of this suite is to verify integration of Java/JNI/C code, 
//  for which running through just one type of archive is enough. 
//  Thus these tests are not replicated for SRA and SRADB 
//  archives, unlike in the C-level test suites
public class ngs_test_CSRA1 {

// ReadCollection
    String PrimaryOnly           = "SRR1063272";
    String WithSecondary         = "SRR833251";
    String WithGroups            = "SRR822962";
    String WithCircularRef       = "SRR1769246";
    String SingleFragmentPerSpot = "SRR2096940";
    
    @Test
    public void open_success() throws ngs.ErrorMsg
    {
        ngs.ReadCollection run = NGS . openReadCollection ( PrimaryOnly );
    }
}
