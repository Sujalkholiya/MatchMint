import React, { useState } from 'react';
import { Match, Competition, SchedulingConflict } from '../../types';
import { Modal } from '../common/Modal';
import { TournamentEngine } from '../../services/tournamentEngine';
import { StorageService } from '../../services/storageService';
import { ConflictAlert } from '../common/ConflictAlert';
import { Clock, MapPin } from 'lucide-react';

interface ScheduleMatchModalProps {
  isOpen: boolean;
  onClose: () => void;
  match: Match | null;
  competition: Competition | null;
  onScheduled: () => void;
}

export const ScheduleMatchModal: React.FC<ScheduleMatchModalProps> = ({
  isOpen,
  onClose,
  match,
  competition,
  onScheduled,
}) => {
  const [scheduledTime, setScheduledTime] = useState<string>(match?.scheduledTime || '');
  const [venue, setVenue] = useState<string>(match?.venue || '');
  const [conflicts, setConflicts] = useState<SchedulingConflict[]>([]);

  React.useEffect(() => {
    if (match) {
      setScheduledTime(match.scheduledTime);
      setVenue(match.venue);
      setConflicts([]);
    }
  }, [match]);

  if (!match || !competition) return null;

  const handleTimeOrVenueChange = (newTime: string, newVenue: string) => {
    setScheduledTime(newTime);
    setVenue(newVenue);

    const matches = StorageService.getMatches().map((m) =>
      m.id === match.id ? { ...m, scheduledTime: newTime, venue: newVenue } : m
    );
    const detected = TournamentEngine.detectSchedulingConflicts(competition, matches);
    setConflicts(detected);
  };

  const handleSave = (e: React.FormEvent) => {
    e.preventDefault();
    StorageService.rescheduleMatch(match.id, scheduledTime, venue);
    onScheduled();
    onClose();
  };

  return (
    <Modal
      isOpen={isOpen}
      onClose={onClose}
      title="Schedule & Venue Assignment"
      subtitle={`Match #${match.matchNumber} (${match.roundName})`}
      maxWidth="md"
    >
      <form onSubmit={handleSave} className="space-y-4">
        {/* Scheduled Time */}
        <div className="space-y-1.5">
          <label className="flex items-center gap-1.5 text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70">
            <Clock className="w-3.5 h-3.5 text-[#D7E2EA]/50" />
            <span>Scheduled Date & Time (YYYY-MM-DD HH:MM)</span>
          </label>
          <input
            type="text"
            required
            value={scheduledTime}
            onChange={(e) => handleTimeOrVenueChange(e.target.value, venue)}
            placeholder="2026-09-25 15:30"
            className="w-full text-xs font-mono p-3 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-white focus:outline-none focus:border-[#B600A8]"
          />
        </div>

        {/* Venue Select */}
        <div className="space-y-1.5">
          <label className="flex items-center gap-1.5 text-xs font-mono uppercase tracking-wider text-[#D7E2EA]/70">
            <MapPin className="w-3.5 h-3.5 text-[#D7E2EA]/50" />
            <span>Allocated Venue / Pitch / Lab</span>
          </label>
          <select
            value={venue}
            onChange={(e) => handleTimeOrVenueChange(scheduledTime, e.target.value)}
            className="w-full text-xs p-3 rounded-2xl bg-[#0C0C0C] border border-[#D7E2EA]/15 text-white focus:outline-none focus:border-[#B600A8]"
          >
            {competition.venues.map((v) => (
              <option key={v} value={v}>
                {v}
              </option>
            ))}
          </select>
        </div>

        {/* Conflict Detection Preview */}
        <div className="pt-1">
          <ConflictAlert conflicts={conflicts} />
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
            Save Schedule
          </button>
        </div>
      </form>
    </Modal>
  );
};
