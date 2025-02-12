/*
 * @BEGIN LICENSE
 *
 * Psi4: an open-source quantum chemistry software package
 *
 * Copyright (c) 2007-2022 The Psi4 Developers.
 *
 * The copyrights for code used from other parties are included in
 * the corresponding files.
 *
 * This file is part of Psi4.
 *
 * Psi4 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * Psi4 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along
 * with Psi4; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * @END LICENSE
 */

/*! \file
    \ingroup CCDENSITY
    \brief Enter brief description of file here
*/
#include <cstdio>
#include "psi4/libdpd/dpd.h"
#include "psi4/libciomr/libciomr.h"
#include "psi4/liboptions/liboptions.h"
#include "MOInfo.h"
#include "Params.h"
#include "Frozen.h"
#define EXTERN
#include "globals.h"

namespace psi {
namespace ccdensity {

void ex_sort_td_rohf(char hand, int Tirrep) {
    int h, nirreps, nmo, nfzv, nfzc, nclsd, nopen;
    int row, col, i, j, I, J, a, b, A, B, p, q;
    double chksum, value;
    psio_address next;
    dpdfile2 D;

    nmo = moinfo.nmo;
    nfzc = moinfo.nfzc;
    nfzv = moinfo.nfzv;
    nclsd = moinfo.nclsd;
    nopen = moinfo.nopen;
    nirreps = moinfo.nirreps;
    const auto& occpi = moinfo.occpi;
    const auto& virtpi = moinfo.virtpi;
    const auto& occ_off = moinfo.occ_off;
    const auto& vir_off = moinfo.vir_off;
    const auto& occ_sym = moinfo.occ_sym;
    const auto& vir_sym = moinfo.vir_sym;
    const auto& openpi = moinfo.openpi;
    const auto& qt_occ = moinfo.qt_occ;
    const auto& qt_vir = moinfo.qt_vir;

    // moinfo.ltd = block_matrix(nmo, nmo);
    double **gtd = block_matrix(nmo, nmo);

    global_dpd_->file2_init(&D, PSIF_CC_TMP, Tirrep, 0, 0, "LTDIJ");
    global_dpd_->file2_mat_init(&D);
    global_dpd_->file2_mat_rd(&D);
    for (h = 0; h < nirreps; h++) {
        for (i = 0; i < occpi[h]; i++) {
            I = qt_occ[occ_off[h] + i];
            for (j = 0; j < occpi[h ^ Tirrep]; j++) {
                J = qt_occ[occ_off[h ^ Tirrep] + j];
                gtd[I][J] += D.matrix[h][i][j];
            }
        }
    }
    global_dpd_->file2_mat_close(&D);
    global_dpd_->file2_close(&D);

    global_dpd_->file2_init(&D, PSIF_CC_TMP, Tirrep, 1, 1, "LTDAB");
    global_dpd_->file2_mat_init(&D);
    global_dpd_->file2_mat_rd(&D);
    for (h = 0; h < nirreps; h++) {
        for (a = 0; a < (virtpi[h] - openpi[h]); a++) {
            A = qt_vir[vir_off[h] + a];
            for (b = 0; b < (virtpi[h ^ Tirrep] - openpi[h ^ Tirrep]); b++) {
                B = qt_vir[vir_off[h ^ Tirrep] + b];
                gtd[A][B] += D.matrix[h][a][b];
            }
        }
    }
    global_dpd_->file2_mat_close(&D);
    global_dpd_->file2_close(&D);

    /* Note that this component of the density is stored occ-vir */
    global_dpd_->file2_init(&D, PSIF_CC_TMP, Tirrep, 0, 1, "LTDAI");
    global_dpd_->file2_mat_init(&D);
    global_dpd_->file2_mat_rd(&D);
    for (h = 0; h < nirreps; h++) {
        for (i = 0; i < occpi[h]; i++) {
            I = qt_occ[occ_off[h] + i];
            for (a = 0; a < (virtpi[h ^ Tirrep] - openpi[h ^ Tirrep]); a++) {
                A = qt_vir[vir_off[h ^ Tirrep] + a];
                gtd[A][I] += D.matrix[h][i][a];
            }
        }
    }
    global_dpd_->file2_mat_close(&D);
    global_dpd_->file2_close(&D);

    global_dpd_->file2_init(&D, PSIF_CC_TMP, Tirrep, 0, 1, "LTDIA");
    global_dpd_->file2_mat_init(&D);
    global_dpd_->file2_mat_rd(&D);
    for (h = 0; h < nirreps; h++) {
        for (i = 0; i < occpi[h]; i++) {
            I = qt_occ[occ_off[h] + i];
            for (a = 0; a < (virtpi[h ^ Tirrep] - openpi[h ^ Tirrep]); a++) {
                A = qt_vir[vir_off[h ^ Tirrep] + a];
                gtd[I][A] += D.matrix[h][i][a];
            }
        }
    }
    global_dpd_->file2_mat_close(&D);
    global_dpd_->file2_close(&D);

    global_dpd_->file2_init(&D, PSIF_CC_TMP, Tirrep, 0, 0, "LTDij");
    global_dpd_->file2_mat_init(&D);
    global_dpd_->file2_mat_rd(&D);
    for (h = 0; h < nirreps; h++) {
        for (i = 0; i < (occpi[h] - openpi[h]); i++) {
            I = qt_occ[occ_off[h] + i];
            for (j = 0; j < (occpi[h ^ Tirrep] - openpi[h ^ Tirrep]); j++) {
                J = qt_occ[occ_off[h ^ Tirrep] + j];
                gtd[I][J] += D.matrix[h][i][j];
            }
        }
    }
    global_dpd_->file2_mat_close(&D);
    global_dpd_->file2_close(&D);

    global_dpd_->file2_init(&D, PSIF_CC_TMP, Tirrep, 1, 1, "LTDab");
    global_dpd_->file2_mat_init(&D);
    global_dpd_->file2_mat_rd(&D);
    for (h = 0; h < nirreps; h++) {
        for (a = 0; a < virtpi[h]; a++) {
            A = qt_vir[vir_off[h] + a];
            for (b = 0; b < virtpi[h ^ Tirrep]; b++) {
                B = qt_vir[vir_off[h ^ Tirrep] + b];
                gtd[A][B] += D.matrix[h][a][b];
            }
        }
    }
    global_dpd_->file2_mat_close(&D);
    global_dpd_->file2_close(&D);

    /* Note that this component of the density is stored occ-vir */
    global_dpd_->file2_init(&D, PSIF_CC_TMP, Tirrep, 0, 1, "LTDai");
    global_dpd_->file2_mat_init(&D);
    global_dpd_->file2_mat_rd(&D);
    for (h = 0; h < nirreps; h++) {
        for (i = 0; i < (occpi[h] - openpi[h]); i++) {
            I = qt_occ[occ_off[h] + i];
            for (a = 0; a < virtpi[h ^ Tirrep]; a++) {
                A = qt_vir[vir_off[h ^ Tirrep] + a];
                gtd[A][I] += D.matrix[h][i][a];
            }
        }
    }
    global_dpd_->file2_mat_close(&D);
    global_dpd_->file2_close(&D);

    global_dpd_->file2_init(&D, PSIF_CC_TMP, Tirrep, 0, 1, "LTDia");
    global_dpd_->file2_mat_init(&D);
    global_dpd_->file2_mat_rd(&D);
    for (h = 0; h < nirreps; h++) {
        for (i = 0; i < (occpi[h] - openpi[h]); i++) {
            I = qt_occ[occ_off[h] + i];
            for (a = 0; a < virtpi[h ^ Tirrep]; a++) {
                A = qt_vir[vir_off[h ^ Tirrep] + a];
                gtd[I][A] += D.matrix[h][i][a];
            }
        }
    }
    global_dpd_->file2_mat_close(&D);
    global_dpd_->file2_close(&D);

    if (hand == 'l') {
        moinfo.ltd = block_matrix(nmo, nmo);
        for (i = 0; i < nmo; ++i)
            for (j = 0; j < nmo; ++j) moinfo.ltd[i][j] = gtd[i][j];
        // print_mat(moinfo.ltd,nmo,nmo,outfile);
    } else if (hand == 'r') {
        moinfo.rtd = block_matrix(nmo, nmo);
        for (i = 0; i < nmo; ++i)
            for (j = 0; j < nmo; ++j) moinfo.rtd[i][j] = gtd[i][j];
        // print_mat(moinfo.rtd,nmo,nmo,outfile);
    } else
        throw PsiException("ccdensity: error", __FILE__, __LINE__);
    free_block(gtd);

    return;
}

}  // namespace ccdensity
}  // namespace psi
