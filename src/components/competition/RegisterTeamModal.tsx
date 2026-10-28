import React, { useState, useEffect } from 'react';
import { Competition } from '../../types';
import { Modal } from '../common/Modal';
import { StorageService } from '../../services/storageService';

interface RegisterTeamModalProps {
  isOpen: boolean;
  onClose: () => void;
  competitions: Competition[];
  initialCompetitionId?: string;
  onRegistered: () => void;
}

export const RegisterTeamModal: React.FC<RegisterTeamModalProps> = ({
  isOpen,
  onClose,
  competitions,
  initialCompetitionId,
  onRegistered,
}) => {
  const [name, setName] = useState('');
  const [captain, setCaptain] = useState('');
  const [membersCount, setMembersCount] = useState('5');
  const [selectedCompId, setSelectedCompId] = useState(
    initialCompetitionId || competitions[0]?.id || ''
  );
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (initialCompetitionId) {
      setSelectedCompId(initialCompetitionId);
    } else if (competitions.length > 0 && !selectedCompId) {
      setSelectedCompId(competitions[0].id);
    }
  }, [initialCompetitionId, competitions]);

  if (!isOpen) return null;

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!name.trim()) {
      setError('Please provide a team name.');
      return;
    }
    if (!captain.trim()) {
      setError('Please provide the captain name.');
      return;
    }
    if (!selectedCompId) {
      setError('Please select a target competition.');
      return;
    }

    const members = parseInt(membersCount, 10) || 5;
    const tag = name.trim().substring(0, 3).toUpperCase();

    try {
      await StorageService.registerTeam({
        competitionId: selectedCompId,
        name: name.trim(),
        captain: captain.trim(),
        email: `${tag.toLowerCase()}.lead@campus.edu`,
        rosterCount: members,
      });

      onRegistered();
      setName('');
      setCaptain('');
      setMembersCount('5');
      setError(null);
      onClose();
    } catch (err: any) {
      setError(err?.message || 'Failed to register team.');
    }
  };

  return (
    <Modal
      isOpen={isOpen}
      onClose={onClose}
      title="Add Team"
      subtitle="Register a new team squad into competition"
      maxWidth="md"
    >
      <form onSubmit={handleSubmit} className="space-y-4">
        {error && (
          <div className="p-3 rounded-2xl bg-rose-500/10 border border-rose-500/30 text-xs text-rose-300 font-mono">
            {error}
          </div>
        )}

        {/* Team Name */}
        <div className="space-y-1.5">
          <label className="text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70">Team Name</label>
          <input
            type="text"
            required
            autoFocus
            value={name}
            onChange={(e) => setName(e.target.value)}
            placeholder="e.g. Falcons"
            className="w-full text-xs p-3 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-white focus:outline-none focus:border-[#B600A8] font-medium"
          />
        </div>

        {/* Captain Name */}
        <div className="space-y-1.5">
          <label className="text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70">Captain Name</label>
          <input
            type="text"
            required
            value={captain}
            onChange={(e) => setCaptain(e.target.value)}
            placeholder="e.g. Aarav Sharma"
            className="w-full text-xs p-3 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-white focus:outline-none focus:border-[#B600A8]"
          />
        </div>

        {/* Members & Competition */}
        <div className="grid grid-cols-2 gap-3">
          <div className="space-y-1.5">
            <label className="text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70">Members</label>
            <input
              type="number"
              min="1"
              max="30"
              value={membersCount}
              onChange={(e) => setMembersCount(e.target.value)}
              placeholder="5"
              className="w-full text-xs font-mono p-3 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-white focus:outline-none focus:border-[#B600A8]"
            />
          </div>

          <div className="space-y-1.5">
            <label className="text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70">Competition</label>
            <select
              value={selectedCompId}
              onChange={(e) => setSelectedCompId(e.target.value)}
              className="w-full text-xs p-3 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-white focus:outline-none focus:border-[#B600A8]"
            >
              {competitions.map((c) => (
                <option key={c.id} value={c.id}>
                  {c.name}
                </option>
              ))}
            </select>
          </div>
        </div>

        {/* Action Buttons */}
        <div className="flex items-center justify-end gap-2.5 pt-4 border-t border-[#D7E2EA]/15">
          <button
            type="button"
            onClick={onClose}
            className="px-4 py-2 text-xs font-mono uppercase tracking-wider rounded-full border-2 border-[#D7E2EA] text-[#D7E2EA] hover:bg-[#D7E2EA]/10 transition-colors cursor-pointer"
          >
            Cancel
          </button>
          <button
            type="submit"
            style={{
              background: 'linear-gradient(123deg, #18011F 7%, #B600A8 37%, #7621B0 72%, #BE4C00 100%)',
              boxShadow: '0px 4px 4px rgba(181, 1, 167, 0.25), 4px 4px 12px #7721B1 inset',
              outline: '2px solid #ffffff',
              outlineOffset: '-3px',
            }}
            className="px-6 py-2 text-xs font-medium uppercase tracking-wider rounded-full text-white hover:opacity-90 transition-opacity cursor-pointer"
          >
            Register Team
          </button>
        </div>
      </form>
    </Modal>
  );
};
