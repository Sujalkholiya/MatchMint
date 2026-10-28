import React, { useState } from 'react';
import { StorageService } from '../services/storageService';
import { Download, Upload, RotateCcw, CheckCircle2 } from 'lucide-react';
import { FadeIn } from '../components/landing/FadeIn';

interface SettingsProps {
  onResetData: () => void;
}

export const Settings: React.FC<SettingsProps> = ({ onResetData }) => {
  const [resetSuccess, setResetSuccess] = useState(false);
  const [importMessage, setImportMessage] = useState<string | null>(null);

  const handleExportJSON = async () => {
    const jsonStr = await StorageService.exportDatabaseJSON();
    const blob = new Blob([jsonStr], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `matchmint_backup_${Date.now()}.json`;
    a.click();
    URL.revokeObjectURL(url);
  };

  const handleImportFile = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = async (event) => {
      const content = event.target?.result as string;
      const success = await StorageService.importDatabaseJSON(content);
      if (success) {
        setImportMessage('Backup restored successfully. Refreshing view...');
        setTimeout(() => {
          window.location.reload();
        }, 800);
      } else {
        setImportMessage('Failed to import backup file. Ensure valid JSON schema.');
      }
    };
    reader.readAsText(file);
  };

  const handleReset = async () => {
    if (window.confirm('Reset all competitions, teams, and matches to default sample data?')) {
      await StorageService.resetToDefaults();
      onResetData();
      setResetSuccess(true);
      setTimeout(() => setResetSuccess(false), 3000);
    }
  };

  return (
    <div className="p-6 max-w-4xl mx-auto space-y-6">
      {/* Header */}
      <FadeIn delay={0} y={20}>
        <div className="pb-2">
          <h1 className="hero-heading text-3xl md:text-5xl font-black uppercase tracking-tight">
            Settings
          </h1>
          <p className="text-xs text-[#D7E2EA]/60 mt-1">
            Data management, migration, and backup controls.
          </p>
        </div>
      </FadeIn>

      {resetSuccess && (
        <div className="p-3.5 rounded-2xl bg-[#18011F] border border-[#B600A8]/40 text-xs text-white flex items-center gap-2 font-mono">
          <CheckCircle2 className="w-4 h-4 text-[#B600A8]" />
          <span>Storage reset to default sample competitions.</span>
        </div>
      )}

      {importMessage && (
        <div className="p-3.5 rounded-2xl bg-[#12151a] border border-[#D7E2EA]/15 text-xs text-[#D7E2EA] font-mono">
          {importMessage}
        </div>
      )}

      {/* 3 Core Actions */}
      <FadeIn delay={0.05} y={20} className="space-y-4">
        {/* Action 1: Export JSON */}
        <div className="p-6 rounded-2xl bg-[#12151a] border border-[#D7E2EA]/15 flex flex-col sm:flex-row sm:items-center justify-between gap-4">
          <div>
            <h3 className="text-base font-bold text-white">Export Backup</h3>
            <p className="text-xs text-[#D7E2EA]/60 mt-1">
              Download all tournaments, teams, fixtures, and audit logs as a single JSON file.
            </p>
          </div>
          <button
            onClick={handleExportJSON}
            className="inline-flex items-center justify-center gap-2 px-5 py-2 text-xs font-mono uppercase tracking-wider rounded-full border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors cursor-pointer shrink-0"
          >
            <Download className="w-4 h-4" />
            <span>Export JSON</span>
          </button>
        </div>

        {/* Action 2: Import JSON */}
        <div className="p-6 rounded-2xl bg-[#12151a] border border-[#D7E2EA]/15 flex flex-col sm:flex-row sm:items-center justify-between gap-4">
          <div>
            <h3 className="text-base font-bold text-white">Import Backup</h3>
            <p className="text-xs text-[#D7E2EA]/60 mt-1">
              Restore complete state from an existing MatchMint JSON backup file.
            </p>
          </div>
          <label className="inline-flex items-center justify-center gap-2 px-5 py-2 text-xs font-mono uppercase tracking-wider rounded-full border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors cursor-pointer shrink-0">
            <Upload className="w-4 h-4" />
            <span>Import JSON</span>
            <input type="file" accept=".json" onChange={handleImportFile} className="hidden" />
          </label>
        </div>

        {/* Action 3: Reset to sample data */}
        <div className="p-6 rounded-2xl bg-[#12151a] border border-[#D7E2EA]/15 flex flex-col sm:flex-row sm:items-center justify-between gap-4">
          <div>
            <h3 className="text-base font-bold text-white">Reset Sample Data</h3>
            <p className="text-xs text-[#D7E2EA]/60 mt-1">
              Clear local modifications and reinitialize the 3 sample tournaments.
            </p>
          </div>
          <button
            onClick={handleReset}
            className="inline-flex items-center justify-center gap-2 px-5 py-2 text-xs font-mono uppercase tracking-wider rounded-full bg-rose-500/10 border border-rose-500/30 text-rose-400 hover:bg-rose-500/20 transition-colors cursor-pointer shrink-0"
          >
            <RotateCcw className="w-3.5 h-3.5" />
            <span>Reset Database</span>
          </button>
        </div>
      </FadeIn>
    </div>
  );
};
